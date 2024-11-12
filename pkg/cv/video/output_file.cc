#include "output_file.h"

#include "core/scope_guard.h"
#include "ffmpeg_error.h"
#include "ffmpeg_filter.h"
#include "ffmpeg_utils.h"
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

// extern int ff_mkdir_p(const char *path);
static const AVClass output_file_class = {
    .class_name = "OutputFile",
    .item_name = av_default_item_name,
    .option = nullptr,
    .version = LIBAVUTIL_VERSION_INT,
};
}

namespace kingfisher {
namespace cv {

OutputFile::OutputFile() : av_class_(&output_file_class) {}
OutputFile::~OutputFile() {
  av_dict_free(&command_opts_);
  av_dict_free(&encoder_opts_);
}

int OutputFile::open(const std::string &filename, AVFormatContext &format_ctx) {
  int ret = 0;
  AVFormatContext *ofmt_ctx = nullptr;
  // std::string fmt = "mp4";
  if ((ret = avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr,
                                            filename.c_str())) < 0) {
    av_log(this, AV_LOG_ERROR,
           "Cannot open output context by file name %s: %s\n", filename.c_str(),
           av_err2str(ret));
    return ret;
  }

  ofmt_ctx_ =
      std::shared_ptr<AVFormatContext>(ofmt_ctx, [](AVFormatContext *ofmt_ctx) {
        if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
          avio_closep(&ofmt_ctx->pb);
        }
        avformat_free_context(ofmt_ctx);
      });

  if (bitexact_) {
    ofmt_ctx->flags |= AVFMT_FLAG_BITEXACT;
  }

  /*
  std::shared_ptr<AVFormatContext> ifmt_ctx =
      std::shared_ptr<AVFormatContext>(&format_ctx);
      */

  ret = create_streams(format_ctx);
  if (ret < 0) {
    av_log(this, AV_LOG_ERROR, "Failed to add streams: %s\n", av_err2str(ret));
    return ret;
  }

  ret = init_filters();
  if (ret < 0) {
    av_log(this, AV_LOG_ERROR, "Failed to open decode filters: %s\n",
           av_err2str(ret));
    return ret;
  }

  if (!(ofmt_ctx_->oformat->flags & AVFMT_NOFILE)) {
    {
      // const char *dir;
      char *fn_copy = av_strdup(ofmt_ctx_->url);
      if (!fn_copy) {
        av_log(this, AV_LOG_ERROR,
               "Failed to allocate the url of output file\n");
        return AVERROR(ENOMEM);
      }
      // dir = av_dirname(fn_copy);
      /*
      if (ff_mkdir_p(dir) == -1 && errno != EEXIST) {
        av_log(ofmt_ctx_.get(), AV_LOG_ERROR,
               "Could not create directory %s with use_localtime_mkdir\n", dir);
        av_freep(&fn_copy);
        return AVERROR(errno);
      }
      */
      av_freep(&fn_copy);
    }
    /* open the file */
    ret = avio_open(&ofmt_ctx_->pb, filename.c_str(), AVIO_FLAG_WRITE);
    if (ret < 0) {
      av_log(this, AV_LOG_ERROR, "Could not open output file '%s': %s\n",
             filename.c_str(), av_err2str(ret));
      return ret;
    }
  }

  /*
   * initialize stream copy and subtitle/data streams.
   * Encoded AVFrame based streams will get initialized as follows:
   * - when the first AVFrame is received in do_video_out
   * - just before the first AVFrame is received in either transcode_step
   *   or reap_filters due to us requiring the filter chain buffer sink
   *   to be configured with the correct audio frame size, which is only
   *   known after the encoder is initialized.
   */
  //  https://sourcegraph.com/github.com/FFmpeg/FFmpeg@release/5.1/-/blob/fftools/ffmpeg.c#:~:text=*/-,for%20(i%20%3D%200%3B%20i%20%3C%20nb_output_streams%3B%20i%2B%2B)%20%7B,-if%20(!
  //
  for (auto &ost : output_streams_) {
    if (!ost->stream_copy_ &&
        (ost->codec_ctx_->codec_type == AVMEDIA_TYPE_VIDEO ||
         ost->codec_ctx_->codec_type == AVMEDIA_TYPE_AUDIO)) {
      continue;
    }

    ret = init_output_stream_wrapper(ost, nullptr);
    if (ret < 0) {
      return ret;
    }
  }

  /* write headers for files with no streams */
  if (ofmt_ctx_->oformat->flags & AVFMT_NOSTREAMS &&
      ofmt_ctx_->nb_streams == 0) {
    ret = of_check_init();
    if (ret < 0) {
      return ret;
    }
  }

  return 0;
}

int OutputFile::init_filter(std::shared_ptr<OutputStream> &ost) {
  AVStream *st = ost->av_stream();
  if (!(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO ||
        st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)) {
    return 0;
  }

  std::string filter_spec;
  if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
    filter_spec = "null"; /* passthrough (dummy) filter for video */
  } else {
    filter_spec = "anull";
  }

  auto fg = std::make_shared<FilterGraph>(ost, filter_spec);
  int ret = fg->init_simple_filtergraph();
  if (ret < 0) {
    av_log(this, AV_LOG_ERROR,
           "Error initializing a simple filtergraph for output stream #%d:%d "
           "failed: %s\n",
           file_index_, ost->stream_index_, av_err2str(ret));
    return ret;
  }
  ost->ofilt_ = fg;
  return 0;
}

int OutputFile::init_filters() {
  int ret = 0;
  for (auto &ost : output_streams_) {
    ret = init_filter(ost);
    if (ret < 0) {
      return ret;
    }
  }

  return 0;
}

int OutputFile::create_streams(const AVFormatContext &format_ctx) {
  int ret = 0;
  output_streams_.resize(ofmt_ctx_->nb_streams);
  for (unsigned int i = 0; i < format_ctx.nb_streams; ++i) {
    AVStream *st = format_ctx.streams[i];
    ret = new_output_stream(format_ctx, i, st->codecpar->codec_type);
    if (ret != 0) {
      av_log(this, AV_LOG_ERROR, "Failed to open decode filters: %s\n",
             av_err2str(ret));
      return ret;
    }
  }

  return 0;
}
/*
int OutputFile::new_output_stream(
    const std::shared_ptr<AVFormatContext> &ifmt_ctx, enum AVMediaType type) {
    */

int OutputFile::new_output_stream(const AVFormatContext &ifmt_ctx,
                                  unsigned int stream_index,
                                  enum AVMediaType type) {
#if 0
  AVStream *ist = ifmt_ctx.streams[stream_index];
  if (ist != nullptr) {
    return -1;
  }
#endif

  AVStream *st = avformat_new_stream(ofmt_ctx_.get(), nullptr);
  if (!st) {
    av_log(this, AV_LOG_ERROR, "Could not alloc output stream -- %s\n",
           av_err2str(AVERROR(ENOMEM)));
    return AVERROR(ENOMEM);
  }

  st->codecpar->codec_type = type;

  if (output_streams_.size() < ofmt_ctx_->nb_streams) {
    output_streams_.resize(ofmt_ctx_->nb_streams);
  }
  std::shared_ptr<OutputStream> ost = std::make_shared<OutputStream>(
      ifmt_ctx, ofmt_ctx_, file_index_,
      stream_index);  // ofmt_ctx_->nb_streams - 1);

  const AVCodec *enc = nullptr;
  int ret = choose_encoder(ost, enc);
  if (ret) {
    av_log(this, AV_LOG_ERROR,
           "Error selecting an encoder for stream "
           "%d:%d\n",
           ost->file_index_, ost->stream_index_);
    return ret;
  }

  auto enc_ctx = std::shared_ptr<AVCodecContext>(
      avcodec_alloc_context3(enc),
      [](AVCodecContext *avctx) { avcodec_free_context(&avctx); });
  if (!enc_ctx) {
    av_log(this, AV_LOG_ERROR,
           "Failed to allocate the encoder context for output stream #%d:%d\n",
           file_index_, ost->stream_index_);
    return AVERROR(ENOMEM);
  }

  ost->codec_ctx_ = enc_ctx;
  enc_ctx->codec_type = type;
  ost->ref_par_ = avcodec_parameters_alloc();
  if (!ost->ref_par_) {
    av_log(this, AV_LOG_ERROR, "Error allocating the encoding parameters.\n");
    return AVERROR(ENOMEM);
  }

  // https://sourcegraph.com/github.com/FFmpeg/FFmpeg@release/5.1/-/blob/doc/examples/transcoding.c?/L177-179#:~:text=if%20(dec_ctx%2D%3Ecodec_type%20%3D%3D%
  if (enc_ctx->codec) {
    if (type == AVMEDIA_TYPE_VIDEO) {
      ost->codec_opts_ = filter_codec_opts(encoder_opts_, enc_ctx->codec->id,
                                           ofmt_ctx_.get(), st, enc_ctx->codec);

      ret = match_per_stream_opt(this, command_opts_, ofmt_ctx_.get(), st,
                                 "autorotate", ost->autorotate_);
      if (ret < 0) {
        return ret;
      }
      ret = match_per_stream_opt(this, command_opts_, ofmt_ctx_.get(), st,
                                 "autoscale", ost->autoscale_);
      if (ret < 0) {
        return ret;
      }

#if 0
    ret = match_per_stream_opt(this, command_opts_, ofmt_ctx_.get(), st,
                               "fpsmax", ost->max_frame_rate_);
    if (ret < 0) {
      return ret;
    }
#endif

      std::string preset;
      ret = match_per_stream_opt(this, command_opts_, ofmt_ctx_.get(), st,
                                 "presets", preset);
      if (ret < 0) {
        return ret;
      }
      if (!preset.empty()) {
        av_log(this, AV_LOG_WARNING,
               "Preset %s specified for stream %d:%d, but not implemented.\n",
               preset.c_str(), ost->file_index_, st->index);
      }

      enc_ctx->width = ost->codec_ctx_->width;
      enc_ctx->height = ost->codec_ctx_->height;
      enc_ctx->sample_aspect_ratio = ost->codec_ctx_->sample_aspect_ratio;
      /* take first format from list of supported formats */
      if (enc->pix_fmts) {
        enc_ctx->pix_fmt = enc->pix_fmts[0];
      } else {
        enc_ctx->pix_fmt = ost->codec_ctx_->pix_fmt;
      }
      /* video time_base can be set to whatever is handy and supported by
       * encoder
       */
      // will be overwritten in init_encoder_time_base later, codec framerate is
      // not reliable
      enc_ctx->time_base = av_inv_q(ost->codec_ctx_->framerate);

      // enc_ctx->gop_size = gop;
      if (enc_ctx->pix_fmt == AV_PIX_FMT_NONE) {
        enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
      }
    }
#if 0
    if (enc_ctx->codec_id == st->codec.codec_id) {
      /* In this example, we transcode to same properties (picture size,
       * sample rate etc.). These properties can be changed for output
       * streams easily using filters */
      enc_ctx->bit_rate = ist->codec.bit_rate;
      // `codec_tag` various with format
      // [mp4 @ 0x162af40] Tag H264 incompatible with output codec id '27'
      // (avc1) enc_ctx->codec_tag = ist->codec.codec_tag;
      enc_ctx->profile = ist->codec.profile;
      enc_ctx->level = ist->codec.level;
    }

#endif
  }

  else {
    ost->codec_opts_ = filter_codec_opts(encoder_opts_, AV_CODEC_ID_NONE,
                                         ofmt_ctx_.get(), st, nullptr);
  }

  if (ifmt_ctx.streams[stream_index]->codecpar) {
    ret = avcodec_parameters_copy(st->codecpar,
                                  ifmt_ctx.streams[stream_index]->codecpar);
    if (ret < 0) {
      av_log(this, AV_LOG_ERROR,
             "Error initializing the output stream codec context for output "
             "stream #%d:%d -- %s\n",
             file_index_, st->index, av_err2str(ret));
      return ret;
    }
  }

  if (bitexact_) {
    ost->codec_ctx_->flags |= AV_CODEC_FLAG_BITEXACT;
  }

  output_streams_[st->index] = ost;

  if (ost->codec_ctx_->codec_type == AVMEDIA_TYPE_VIDEO) {
    video_stream_index_ = st->index;
  } else if (ost->codec_ctx_->codec_type == AVMEDIA_TYPE_AUDIO) {
    audio_stream_index_ = st->index;
  } else if (ost->codec_ctx_->codec_type == AVMEDIA_TYPE_UNKNOWN) {
    av_log(this, AV_LOG_ERROR,
           "Elementary stream #%d:%d is of unknown type, cannot proceed\n",
           file_index_, st->index);
    return AVERROR_INVALIDDATA;
  }

  return 0;
}

int OutputFile::choose_encoder(const std::shared_ptr<OutputStream> &ost,
                               const AVCodec *&codec) {
  std::string codec_name;
  auto &st = ost->st_;
  ost->encoding_needed_ = true;
  int ret = match_per_stream_opt(this, command_opts_, ofmt_ctx_.get(), st, "c",
                                 codec_name);
  if (ret != 0) {
    return ret;
  }

  AVMediaType type = st->codecpar->codec_type;
  if (type == AVMEDIA_TYPE_VIDEO || type == AVMEDIA_TYPE_AUDIO ||
      type == AVMEDIA_TYPE_SUBTITLE) {
    if (codec_name.empty()) {
      st->codecpar->codec_id =
          av_guess_codec(ofmt_ctx_->oformat, nullptr, ofmt_ctx_->url, nullptr,
                         st->codecpar->codec_type);
      codec = avcodec_find_encoder(st->codecpar->codec_id);
      if (!codec) {
        av_log(NULL, AV_LOG_FATAL,
               "Automatic encoder selection failed for "
               "output stream #%d:%d. Default encoder for format %s (codec "
               "%s) is "
               "probably disabled. Please choose an encoder manually.\n",
               ost->file_index_, ost->stream_index_, ofmt_ctx_->oformat->name,
               avcodec_get_name(st->codecpar->codec_id));
        return AVERROR_ENCODER_NOT_FOUND;
      }
      ost->stream_copy_ = false;
    } else if (codec_name == "copy") {
      ost->stream_copy_ = true;
    } else {
      ost->stream_copy_ = false;
      ret = find_encoder(codec_name, st->codecpar->codec_type,

                         codec);
      if (ret) {
        return ret;
      }
      st->codecpar->codec_id = codec->id;
      if (recast_media_ && st->codecpar->codec_type != codec->type) {
        st->codecpar->codec_type = codec->type;
      }
    }
  } else {
    ost->stream_copy_ = true;
  }
  ost->encoding_needed_ = !ost->stream_copy_;

  return 0;
}

int OutputFile::find_encoder(const std::string &name, enum AVMediaType type,
                             const AVCodec *&codec) const {
  return find_codec((void *)this, name, type, true, codec, recast_media_);
}

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
  const auto &ist = ost->input_av_stream();
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
  const auto &ist = ost->input_av_stream();
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

  // = 0 : set default timebase (1/(frame_rate or sample_rate in input
  // stream)) for encode_context
  enc_ctx->time_base = default_time_base;
}

int OutputFile::init_output_stream(const std::shared_ptr<OutputStream> &ost,
                                   AVFrame *frame) {
  int ret = 0;
  if (ost->encoding_needed_) {
    if (bitexact_) {
      ost->codec_ctx_->flags |= AV_CODEC_FLAG_BITEXACT;
    }

    if (ofmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER) {
      ost->codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    ret = init_output_stream_encode(ost, frame);
    if (ret < 0) {
      return ret;
    }

    if (!av_dict_get(ost->codec_opts_, "threads", nullptr, 0)) {
      av_dict_set(&ost->codec_opts_, "threads", "auto", 0);
    }

    if (!av_dict_get(ost->codec_opts_, "preset", nullptr, 0)) {
      if (ost->codec_ctx_->codec_type == AVMEDIA_TYPE_VIDEO) {
        av_dict_set(&ost->codec_opts_, "preset", "fast",
                    AV_DICT_DONT_OVERWRITE);
      }
    }

    if (!av_dict_get(ost->codec_opts_, "zerolatency", nullptr, 0)) {
      if (ost->codec_ctx_->codec_type == AVMEDIA_TYPE_VIDEO) {
        av_dict_set(&ost->codec_opts_, "zerolatency", "true",
                    AV_DICT_DONT_OVERWRITE);
      }
    }

    if (ost->codec_ctx_->codec_id == AV_CODEC_ID_H264) {
      if (!strcmp(ost->codec_ctx_->codec->name, "h264_nvenc")) {
        if (!av_dict_get(ost->codec_opts_, "tune", nullptr, 0)) {
          av_dict_set(&ost->codec_opts_, "tune", "ll", AV_DICT_DONT_OVERWRITE);
        }
      } else {
        if (!av_dict_get(ost->codec_opts_, "tune", nullptr, 0)) {
          av_dict_set(&ost->codec_opts_, "tune", "zerolatency",
                      AV_DICT_DONT_OVERWRITE);
        }
      }
    } else if (ost->codec_ctx_->codec_id == AV_CODEC_ID_HEVC) {
      if (!strcmp(ost->codec_ctx_->codec->name, "hevc_nvenc")) {
        if (!av_dict_get(ost->codec_opts_, "tune", nullptr, 0)) {
          av_dict_set(&ost->codec_opts_, "tune", "ll", AV_DICT_DONT_OVERWRITE);
        }
      } else {
        if (!av_dict_get(ost->codec_opts_, "tune", nullptr, 0)) {
          av_dict_set(&ost->codec_opts_, "tune", "zerolatency",
                      AV_DICT_DONT_OVERWRITE);
        }
      }
    }

    // todo hw_device_setup_for_encode
    const AVCodec *codec = ost->codec_ctx_->codec;
    auto st = ost->av_stream();
    ret = avcodec_open2(ost->codec_ctx_.get(), codec, &ost->codec_opts_);
    if (ret < 0) {
      av_log(this, AV_LOG_ERROR,
             "Error while opening encoder for output stream #%d:%d -- %s - "
             "maybe incorrect parameters such as bit_rate, rate, width or "
             "height\n",
             file_index_, ost->stream_index_, av_err2str(ret));
      return ret;
    }
    if (codec->type == AVMEDIA_TYPE_AUDIO &&
        !(codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)) {
      AVFilterContext *filter_ctx =
          !ost->ofilt_ || ost->ofilt_->outputs_.empty()
              ? nullptr
              : ost->ofilt_->outputs_[0]->filter_;
      if (filter_ctx) {
        av_buffersink_set_frame_size(filter_ctx, ost->codec_ctx_->frame_size);
      }
    }

    // assert_avoptions(ost->codec_opts_);
    if (ost->codec_ctx_->bit_rate && ost->codec_ctx_->bit_rate < 1000 &&
        ost->codec_ctx_->codec_id !=
            AV_CODEC_ID_CODEC2 /* don't complain about 700 bit/s modes */) {
      av_log(this, AV_LOG_WARNING,
             "The bitrate parameter is set too low."
             " It takes bits/s as argument, not kbits/s\n");
    }

    ret = avcodec_parameters_from_context(st->codecpar, ost->codec_ctx_.get());
    if (ret < 0) {
      av_log(this, AV_LOG_ERROR,
             "Error initializing the output stream codec context for output "
             "stream #%d:%d -- %s\n",
             file_index_, st->index, av_err2str(ret));
      return ret;
    }

    if (ost->codec_ctx_->nb_coded_side_data) {
      int i;
      for (i = 0; i < ost->codec_ctx_->nb_coded_side_data; i++) {
        const AVPacketSideData *sd_src = &ost->codec_ctx_->coded_side_data[i];
        uint8_t *dst_data;

        dst_data = av_stream_new_side_data(st, sd_src->type, sd_src->size);
        if (!dst_data) {
          return AVERROR(ENOMEM);
        }
        memcpy(dst_data, sd_src->data, sd_src->size);
      }
    }

    /*
     * Add global input side data. For now this is naive, and copies it
     * from the input stream's global side data. All side data should
     * really be funneled over AVFrame and libavfilter, then added back to
     * packet side data, and then potentially using the first packet for
     * global side data.
     */

    // copy timebase while removing common factors
    if (st->time_base.num <= 0 || st->time_base.den <= 0) {
      st->time_base = av_add_q(ost->codec_ctx_->time_base, (AVRational){0, 1});
    }

    // copy estimated duration as a hint to the muxer
    if (st->duration <= 0 && ost->st_ && ost->st_->duration > 0) {
      st->duration =
          av_rescale_q(ost->st_->duration, ost->st_->time_base, st->time_base);
    }
  } else if (ost->stream_copy_) {
    ret = init_output_stream_streamcopy(ost);
    if (ret < 0) {
      return ret;
    }
  }

  /* initialize bitstream filters for the output stream
   * needs to be done here, because the codec id for streamcopy is not
   * known until now */
#if 0
  ret = init_output_bsfs(ost);
  if (ret < 0) return ret;

  ost->initialized = true;

  ret = of_check_init(output_files[ost->file_index]);
  if (ret < 0) return ret;
#endif
  ost->initialized_ = true;
  ret = of_check_init();
  if (ret < 0) {
    return ret;
  }
  // ost->initialized_ = true;

  return 0;
}

int OutputFile::init_output_stream_streamcopy(
    const std::shared_ptr<OutputStream> &ost) {
  const auto &ist = ost->input_av_stream();
  const auto &st = ost->av_stream();
  AVCodecParameters *par_dst = st->codecpar;
  AVCodecParameters *par_src = ost->ref_par_;
  AVRational sar;
  uint32_t codec_tag = par_dst->codec_tag;
  int ret = 0;
  av_assert0(ist && !ost->filter_);

  ret = avcodec_parameters_from_context(par_src, ost->codec_ctx_.get());
  if (ret < 0) {
    av_log(NULL, AV_LOG_FATAL, "Error getting reference codec parameters.\n");
    return ret;
  }

  const AVOutputFormat *oformat = ofmt_ctx_->oformat;
  if (!codec_tag) {
    unsigned int codec_tag_tmp;
    if (!oformat->codec_tag ||
        av_codec_get_id(oformat->codec_tag, par_src->codec_tag) ==
            par_src->codec_id ||
        !av_codec_get_tag2(oformat->codec_tag, par_src->codec_id,
                           &codec_tag_tmp)) {
      codec_tag = par_src->codec_tag;
    }
  }

  ret = avcodec_parameters_copy(par_dst, par_src);
  if (ret < 0) {
    return ret;
  }

  if (!ost->framerate_.num) {
    ost->framerate_ = ist->r_frame_rate;
  }

  if (ost->framerate_.num) {
    st->avg_frame_rate = ost->framerate_;
  } else {
    st->avg_frame_rate = ist->avg_frame_rate;
  }

  // todo avformat_transfer_internal_stream_timing_info
  ret = avformat_transfer_internal_stream_timing_info(oformat, st, ist,
                                                      AVFMT_TBCF_AUTO);
  if (ret < 0) {
    return ret;
  }

  if (ist->time_base.den) {
    st->time_base = ist->time_base;
  }

  // copy timebase while removing common factors
  if (st->time_base.num <= 0 || st->time_base.den <= 0) {
    if (ost->frame_rate.num) {
      st->time_base = av_add_q(ost->codec_ctx_->time_base, (AVRational){0, 1});
    } else {
      st->time_base =
          av_add_q(av_stream_get_codec_timebase(st), (AVRational){0, 1});
    }
  }

  // copy estimated duration as a hint to the muxer
  if (st->duration <= 0 && ost->st_ && ost->st_->duration > 0) {
    st->duration =
        av_rescale_q(ost->st_->duration, ost->st_->time_base, st->time_base);
  }

  switch (par_dst->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
      if ((par_dst->block_align == 1 || par_dst->block_align == 1152 ||
           par_dst->block_align == 576) &&
          par_dst->codec_id == AV_CODEC_ID_MP3) {
        par_dst->block_align = 0;
      }
      if (par_dst->codec_id == AV_CODEC_ID_AC3) {
        par_dst->block_align = 0;
      }
      break;
    case AVMEDIA_TYPE_VIDEO:
      if (ist->sample_aspect_ratio
              .num) {  // overridden by the -aspect cli option
        sar = av_mul_q(ist->sample_aspect_ratio,
                       (AVRational){par_dst->height, par_dst->width});
        av_log(this, AV_LOG_WARNING,
               "Overriding aspect ratio "
               "with stream copy may produce invalid files\n");
      } else if (st->sample_aspect_ratio.num) {
        sar = st->sample_aspect_ratio;
      } else {
        sar = par_src->sample_aspect_ratio;
      }
      st->sample_aspect_ratio = par_dst->sample_aspect_ratio = sar;
      st->avg_frame_rate = ist->avg_frame_rate;
      st->r_frame_rate = ist->r_frame_rate;
      break;
    default:
      break;
  }

  return 0;
}

/* open the muxer when all the streams are initialized */
int OutputFile::of_check_init() {
#if 0
  for (unsigned int i = 0; i < ofmt_ctx_->nb_streams; i++) {
    const auto &ost = output_streams_[i];
    if (!ost->initialized_) {
      return 0;
    }
  }
#endif

  int ret = avformat_write_header(ofmt_ctx_.get(), &opts_);
  if (ret < 0) {
    av_log(this, AV_LOG_ERROR,
           "Could not write header for output file #%d "
           "(incorrect codec parameters ?): %s\n",
           file_index_, av_err2str(ret));
    return ret;
  }
  header_written_ = true;

  av_dump_format(ofmt_ctx_.get(), file_index_, ofmt_ctx_->url, 1);

  /* flush the muxing queues */
  for (auto ib = muxing_queue_.begin(); ib != muxing_queue_.end();) {
    const auto &pkt = *ib;
    if (pkt->stream_index >= 0 &&
        pkt->stream_index < static_cast<int>(output_streams_.size())) {
      const auto &ost = output_streams_[pkt->stream_index];
      ret = of_write_packet(ost, pkt.get());
      if (ret < 0) {
        return ret;
      }
    }
    ib = muxing_queue_.erase(ib);
  }
  return 0;
}

int OutputFile::of_write_packet(const std::shared_ptr<OutputStream> &ost,
                                AVPacket *pkt) {
  const auto &s = ofmt_ctx_;
  AVStream *st = ost->av_stream();
  int ret = 0;
  if (!pkt) {
    return 0;
  }

  if (!header_written_) {
    /* the muxer is not initialized yet, buffer the packet */
    muxing_queue_.push_back(std::shared_ptr<AVPacket>(
        av_packet_clone(pkt), [](AVPacket *pkt) { av_packet_free(&pkt); }));
    return 0;
  }

  pkt->stream_index = static_cast<int>(st->index);
  av_packet_rescale_ts(pkt, ost->mux_timebase_, st->time_base);
  pkt->time_base = st->time_base;

  if (!(s->oformat->flags & AVFMT_NOTIMESTAMPS)) {
    if (pkt->dts != AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE &&
        pkt->dts > pkt->pts) {
      av_log(s.get(), AV_LOG_WARNING,
             "Invalid DTS: %" PRId64 " PTS: %" PRId64
             " in output stream %d:%d, replacing by guess\n",
             pkt->dts, pkt->pts, ost->file_index_, ost->av_stream()->index);
      pkt->pts = pkt->dts = pkt->pts + pkt->dts + ost->last_mux_dts_ + 1 -
                            FFMIN3(pkt->pts, pkt->dts, ost->last_mux_dts_ + 1) -
                            FFMAX3(pkt->pts, pkt->dts, ost->last_mux_dts_ + 1);
    }
    if ((st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO ||
         st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ||
         st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) &&
        pkt->dts != AV_NOPTS_VALUE && ost->last_mux_dts_ != AV_NOPTS_VALUE) {
      int64_t max =
          ost->last_mux_dts_ + !(s->oformat->flags & AVFMT_TS_NONSTRICT);
      if (pkt->dts < max) {
        int loglevel =
            max - pkt->dts > 2 || st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO
                ? AV_LOG_WARNING
                : AV_LOG_DEBUG;

        av_log(s.get(), loglevel,
               "Non-monotonous DTS in output stream "
               "%d:%d; previous: %" PRId64 ", current: %" PRId64 "; ",
               ost->file_index_, ost->av_stream()->index, ost->last_mux_dts_,
               pkt->dts);
        av_log(s.get(), loglevel,
               "changing to %" PRId64
               ". This may result "
               "in incorrect timestamps in the output file.\n",
               max);
        if (pkt->pts >= pkt->dts) {
          pkt->pts = FFMAX(pkt->pts, max);
        }
        pkt->dts = max;
      }
    }
  }

  ost->last_mux_dts_ = pkt->dts;
  ost->data_size_ += pkt->size;
  ost->packets_written_++;

  // pkt->stream_index = static_cast<int>(st->index);

  if (debug_ts_) {
    av_log(NULL, AV_LOG_INFO,
           "muxer <- type:%s "
           "pkt_pts:%s pkt_pts_time:%s pkt_dts:%s pkt_dts_time:%s duration:%s "
           "duration_time:%s size:%d\n",
           av_get_media_type_string(ost->codec_ctx_->codec_type),
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, &st->time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, &st->time_base),
           av_ts2str(pkt->duration),
           av_ts2timestr(pkt->duration, &st->time_base), pkt->size);
  }

  ret = av_interleaved_write_frame(ofmt_ctx_.get(), pkt);
  if (ret < 0) {
    av_log(this, AV_LOG_ERROR,
           "av_interleaved_write_frame failed for output stream #%d:%d -- %s\n",
           file_index_, st->index, av_err2str(ret));
    return ret;
  }

  return 0;
}

int OutputFile::write_packet(const std::shared_ptr<AVPacket> &enc_pkt,
                             std::shared_ptr<OutputStream> &ost) {
  int ret = 0;
  if (!ost->initialized_) {
    ost->stream_copy_ = true;       // remux for write_packet
    ost->encoding_needed_ = false;  // remux for write_packet
  }
  ret = init_output_stream_wrapper(ost, nullptr);
  if (ret < 0) {
    return ret;
  }

  ret = of_write_packet(ost, enc_pkt.get());
  if (ret < 0) {
    return ret;
  }
  return 0;
}

int OutputFile::write_frame(int stream_index, const Frame &frame) {
  auto &ost = output_streams_[stream_index];
  if (frame.packet) {
    return write_packet(frame.packet, ost);
  }

  return 0;
}

int OutputFile::write_frame(const Frame &raw_frame) {
  switch (raw_frame.codec_type) {
    case AVMEDIA_TYPE_VIDEO:
      if (video_stream_index_ >= 0) {
        return write_frame(video_stream_index_, raw_frame);
      }
      break;
    case AVMEDIA_TYPE_AUDIO:
      if (audio_stream_index_ >= 0) {
        return write_frame(audio_stream_index_, raw_frame);
      }
      break;
    default:
      break;
  }
  return 0;
}

int OutputFile::write_frames(const std::vector<Frame> &raw_frames) {
  int ret = 0;
  for (const auto &raw_frame : raw_frames) {
    if ((ret = write_frame(raw_frame)) < 0) {
      return ret;
    }
  }
  return 0;
}

}  // namespace cv
}  // namespace kingfisher
