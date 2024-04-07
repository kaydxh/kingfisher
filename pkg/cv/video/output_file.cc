#include "output_file.h"

#include "core/scope_guard.h"
#include "ffmpeg_error.h"
#include "ffmpeg_filter.h"
#include "output_filter.h"
#include "output_stream.h"

extern "C" {
#include "libavfilter/buffersink.h"
#include "libavutil/avassert.h"
#include "libavutil/bprint.h"
#include "libavutil/channel_layout.h"
#include "libavutil/display.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
}

namespace kingfisher {
namespace cv {

static const AVClass output_file_class = {
    .class_name = "OutputFile",
    .item_name = av_default_item_name,
    .option = nullptr,
    .version = LIBAVUTIL_VERSION_INT,
};

OutputFile::OutputFile() : av_class_(&output_file_class) {}
OutputFile::~OutputFile() {}

int OutputFile::init_output_stream_wrapper(
    const std::shared_ptr<OutputStream> &ost, AVFrame *frame) {
  int ret = AVERROR_BUG;

  if (ost->initialized_) {
    return 0;
  }

  ret = init_output_stream(ost, frame);
  if (ret < 0) {
    av_log(nullptr, AV_LOG_ERROR,
           "Error initializing output stream %d:%d -- %s\n", ost->file_index_,
           ost->stream_index_, av_err2str(ret));
    return ret;
  }

  return ret;
}

int OutputFile::init_output_stream_encode(
    const std::shared_ptr<OutputStream> &ost, AVFrame *frame) {
  const auto &ist = ost->st_;
  const auto enc = ost->codec_ctx_->codec;
  const auto &filter_ctx = ost->filter_->filter_;
  AVCodecContext *enc_ctx = ost->codec_ctx_.get();
  int ret = 0;
  set_encoder_id(ost);

  if (enc_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
    if (!ost->framerate_.num) {
      ost->framerate_ = av_buffersink_get_frame_rate(filter_ctx);
    }

    if (ist && !ost->framerate_.num) {
      if (ist->r_frame_rate.num) {
        ost->framerate_ = ist->r_frame_rate;
      } else if (ist->avg_frame_rate.num) {
        ost->framerate_ = ist->avg_frame_rate;
      }
    }

    if (ist && !ost->framerate_.num && !ost->max_frame_rate_.num) {
      ost->framerate_ = (AVRational){25, 1};
      av_log(nullptr, AV_LOG_WARNING,
             "No information "
             "about the input framerate is available. Falling "
             "back to a default value of 25fps for output stream #%d:%d. Use "
             "the -r option "
             "if you want a different framerate.\n",
             ost->file_index_, ost->stream_index_);
    }

    if (ost->max_frame_rate_.num &&
        (av_q2d(ost->framerate_) > av_q2d(ost->max_frame_rate_) ||
         !ost->framerate_.den)) {
      ost->framerate_ = ost->max_frame_rate_;
    }

    if (enc->supported_framerates && !ost->force_fps_) {
      int idx =
          av_find_nearest_q_idx(ost->framerate_, enc->supported_framerates);
      ost->framerate_ = enc->supported_framerates[idx];
    }
    // reduce frame rate for mpeg4 to be within the spec limits
    if (enc_ctx->codec_id == AV_CODEC_ID_MPEG4) {
      av_reduce(&ost->framerate_.num, &ost->framerate_.den, ost->framerate_.num,
                ost->framerate_.den, 65535);
    }
  }

  switch (enc_ctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
      enc_ctx->sample_fmt =
          static_cast<AVSampleFormat>(av_buffersink_get_format(filter_ctx));
      enc_ctx->sample_rate = av_buffersink_get_sample_rate(filter_ctx);
      ret = av_buffersink_get_ch_layout(filter_ctx, &enc_ctx->ch_layout);
      if (ret < 0) {
        return ret;
      }

      init_encoder_time_base(ost, av_make_q(1, enc_ctx->sample_rate));
      break;

    case AVMEDIA_TYPE_VIDEO:
      init_encoder_time_base(ost, av_inv_q(ost->framerate_));
      if (filter_ctx && !(enc_ctx->time_base.num && enc_ctx->time_base.den)) {
        enc_ctx->time_base = av_buffersink_get_time_base(filter_ctx);
      }

      if (filter_ctx) {
        enc_ctx->width = av_buffersink_get_w(filter_ctx);
        enc_ctx->height = av_buffersink_get_h(filter_ctx);
        enc_ctx->sample_aspect_ratio = ost->st_->sample_aspect_ratio =
            av_buffersink_get_sample_aspect_ratio(filter_ctx);

        enc_ctx->pix_fmt =
            static_cast<AVPixelFormat>(av_buffersink_get_format(filter_ctx));
      }

      if (ost->bits_per_raw_sample_) {
        enc_ctx->bits_per_raw_sample = ost->bits_per_raw_sample_;
      } else if (ost->ofilt_ && ost->ofilt_->is_meta_) {
        enc_ctx->bits_per_raw_sample =
            av_pix_fmt_desc_get(enc_ctx->pix_fmt)->comp[0].depth;
      }

      if (frame) {
        enc_ctx->color_range = frame->color_range;
        enc_ctx->color_primaries = frame->color_primaries;
        enc_ctx->color_trc = frame->color_trc;
        enc_ctx->colorspace = frame->colorspace;
        enc_ctx->chroma_sample_location = frame->chroma_location;
      }

      enc_ctx->framerate = ost->framerate_;

      ost->st_->avg_frame_rate = ost->framerate_;

      // Field order: autodetection
      if (frame) {
        if (enc_ctx->flags &
                (AV_CODEC_FLAG_INTERLACED_DCT | AV_CODEC_FLAG_INTERLACED_ME) &&
            ost->top_field_first_ >= 0) {
          frame->top_field_first = !!ost->top_field_first_;
        }

        if (frame->interlaced_frame) {
          if (enc_ctx->codec->id == AV_CODEC_ID_MJPEG) {
            enc_ctx->field_order =
                frame->top_field_first ? AV_FIELD_TT : AV_FIELD_BB;
          } else {
            enc_ctx->field_order =
                frame->top_field_first ? AV_FIELD_TB : AV_FIELD_BT;
          }
        } else {
          enc_ctx->field_order = AV_FIELD_PROGRESSIVE;
        }
      }

      // Field order: override
      if (ost->top_field_first_ == 0) {
        enc_ctx->field_order = AV_FIELD_BB;
      } else if (ost->top_field_first_ == 1) {
        enc_ctx->field_order = AV_FIELD_TT;
      }
      break;
    case AVMEDIA_TYPE_SUBTITLE:
      enc_ctx->time_base = AV_TIME_BASE_Q;
      if (!enc_ctx->width) {
        if (ist->codecpar) {
          enc_ctx->width = ist->codecpar->width;
          enc_ctx->height = ist->codecpar->height;
        }
      }
      break;
    default:
      break;
  }

  return 0;
}

void OutputFile::set_encoder_id(const std::shared_ptr<OutputStream> &ost) {
  const AVDictionaryEntry *e = nullptr;

  int format_flags = 0;
  int codec_flags = ost->codec_ctx_->flags;

  const auto &st = ost->av_stream();
  const auto &codec = ost->codec_ctx_->codec;

  if (av_dict_get(st->metadata, "encoder", nullptr, 0)) {
    return;
  }

  e = av_dict_get(opts_, "fflags", nullptr, 0);
  if (e) {
    const AVOption *o = av_opt_find(ofmt_ctx_.get(), "fflags", nullptr, 0, 0);
    if (!o) {
      return;
    }
    av_opt_eval_flags(ofmt_ctx_.get(), o, e->value, &format_flags);
  }
  e = av_dict_get(ost->codec_opts_, "flags", nullptr, 0);
  if (e) {
    const AVOption *o =
        av_opt_find(ost->codec_ctx_.get(), "flags", nullptr, 0, 0);
    if (!o) {
      return;
    }
    av_opt_eval_flags(ost->codec_ctx_.get(), o, e->value, &codec_flags);
  }

  AVBPrint args;
  av_bprint_init(&args, 0, AV_BPRINT_SIZE_AUTOMATIC);
  SCOPE_EXIT { av_bprint_finalize(&args, nullptr); };

  if (!(format_flags & AVFMT_FLAG_BITEXACT) &&
      !(codec_flags & AV_CODEC_FLAG_BITEXACT)) {
    av_bprintf(&args, LIBAVCODEC_IDENT " %s", codec ? codec->name : "copy");

  } else {
    av_bprintf(&args, "Lavc %s", codec ? codec->name : "copy");
  }
  av_dict_set(&st->metadata, "encoder", args.str, AV_DICT_DONT_OVERWRITE);

  return;
}

void OutputFile::init_encoder_time_base(
    const std::shared_ptr<OutputStream> &ost, AVRational default_time_base) {
  const auto &ist = ost->st_;
  auto enc_ctx = ost->codec_ctx_;

  // > 0 : set as this timebase for encode_context
  if (ost->enc_timebase_.num > 0) {
    enc_ctx->time_base = ost->enc_timebase_;
    return;
  }
  // < 0 : set output stream timebase for encode_context
  if (ost->enc_timebase_.num < 0) {
    if (ist && ist->time_base.num) {
      enc_ctx->time_base = ist->time_base;
      return;
    }
    av_log(this, AV_LOG_WARNING,
           "Input stream data not available, using default time base\n");
  }

  // = 0 : set default timebase (1/(frame_rate or sample_rate in input stream))
  // for encode_context
  enc_ctx->time_base = default_time_base;
}

}  // namespace cv
}  // namespace kingfisher
