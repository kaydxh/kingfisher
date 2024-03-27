#include "input_filter.h"

#include <iostream>

#include "core/scope_guard.h"
#include "ffmpeg_error.h"
#include "ffmpeg_filter.h"
#include "stream.h"

extern "C" {
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavformat/avformat.h"
#include "libavutil/avassert.h"
#include "libavutil/bprint.h"
#include "libavutil/fifo.h"
#include "libavutil/pixdesc.h"

static const AVClass input_filter_class = {
    .class_name = "Input Filter",
    .item_name = av_default_item_name,
    .option = nullptr,
    .version = LIBAVUTIL_VERSION_INT,
};
}

namespace kingfisher {
namespace cv {

InputFilter::InputFilter(const std::shared_ptr<FilterGraph> &fg,
                         const std::weak_ptr<Stream> &ist)
    : av_class_(&input_filter_class), graph_(fg), ist_(ist) {
  frame_queue_ = std::shared_ptr<AVFifo>(
      av_fifo_alloc2(8, sizeof(AVFifo *), AV_FIFO_FLAG_AUTO_GROW),
      [](AVFifo *p) { av_fifo_freep2(&p); });
}

InputFilter::~InputFilter() { av_freep(&display_matrix_); }

int InputFilter::ifilter_send_frame(const std::shared_ptr<AVFrame> &frame,
                                    int keep_reference) {
  int buffersrc_flags = AV_BUFFERSRC_FLAG_PUSH;
  if (keep_reference) {
    buffersrc_flags |= AV_BUFFERSRC_FLAG_KEEP_REF;
  }

  const auto &ist = ist_.lock();
  if (!ist) {
    return -1;
  }
  auto st = ist->av_stream();
  if (!st) {
    return -1;
  }

  const auto &fg = graph_.lock();

  int need_reinit = format_ != frame->format;
  switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
      need_reinit |= sample_rate_ != frame->sample_rate ||
                     av_channel_layout_compare(&ch_layout_, &frame->ch_layout);
      break;
    case AVMEDIA_TYPE_VIDEO:
      need_reinit |= width_ != frame->width || height_ != frame->height;
      break;

    default:
      break;
  }

  if (!ist->reinit_filters_ && fg->filter_graph_) {
    need_reinit = false;
  }

  if ((hw_frames_ctx_ != nullptr) != (frame->hw_frames_ctx != nullptr) ||
      (hw_frames_ctx_ && hw_frames_ctx_->data != frame->hw_frames_ctx->data)) {
    need_reinit = true;
  }

  AVFrameSideData *sd = nullptr;
  if ((sd = av_frame_get_side_data(frame.get(), AV_FRAME_DATA_DISPLAYMATRIX))) {
    if (!display_matrix_ ||
        memcmp(sd->data, display_matrix_, sizeof(int32_t) * 9) != 0) {
      need_reinit = true;
    }
  } else if (display_matrix_) {
    need_reinit = true;
  }

  int ret = 0;
  if (need_reinit) {
    ret = ifilter_parameters_from_frame(frame);
    if (ret < 0) {
      return ret;
    }
  }

  /* (re)init the graph if possible, otherwise buffer the frame and return */
  if (need_reinit || !fg->filter_graph_) {
    if (!ifilter_has_input_format()) {
      AVFrame *tmp = av_frame_clone(frame.get());
      if (!tmp) {
        return AVERROR(ENOMEM);
      }
      ret = av_fifo_write(frame_queue_.get(), &tmp, 1);
      if (ret < 0) {
        av_frame_free(&tmp);
      }

      return ret;
    }

    ret = fg->reap_filters();
    if (ret < 0 && ret != AVERROR_EOF) {
      av_log(this, AV_LOG_ERROR, "Error while filtering: %s\n",
             av_err2str(ret));
      return ret;
    }
    ret = fg->configure_filtergraph();
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR, "Error reinitializing filters!: %s\n",
             av_err2str(ret));
      return ret;
    }
  }

  ret = av_buffersrc_add_frame_flags(filter_, frame.get(), buffersrc_flags);
  if (ret < 0) {
    if (ret != AVERROR_EOF)
      av_log(this, AV_LOG_ERROR, "Error while filtering: %s\n",
             av_err2str(ret));
    return ret;
  }

  return 0;
}

int InputFilter::ifilter_parameters_from_frame(
    const std::shared_ptr<AVFrame> &frame) {
  AVFrameSideData *sd = nullptr;
  int ret = 0;

  av_buffer_unref(&hw_frames_ctx_);

  format_ = frame->format;

  width_ = frame->width;
  height_ = frame->height;
  sample_aspect_ratio_ = frame->sample_aspect_ratio;

  sample_rate_ = frame->sample_rate;
  ret = av_channel_layout_copy(&ch_layout_, &frame->ch_layout);
  if (ret < 0) {
    return ret;
  }

  av_freep(&display_matrix_);
  sd = av_frame_get_side_data(frame.get(), AV_FRAME_DATA_DISPLAYMATRIX);
  if (sd) {
    display_matrix_ =
        static_cast<int32_t *>(av_memdup(sd->data, sizeof(int32_t) * 9));
  }

  if (frame->hw_frames_ctx) {
    hw_frames_ctx_ = av_buffer_ref(frame->hw_frames_ctx);
    if (!hw_frames_ctx_) {
      return AVERROR(ENOMEM);
    }
  }

  return 0;
}

bool InputFilter::ifilter_has_input_format() const {
  return !(format_ < 0 &&
           (type_ == AVMEDIA_TYPE_AUDIO || type_ == AVMEDIA_TYPE_VIDEO));
}

int InputFilter::configure_input_filter(AVFilterInOut *in) {
  auto const &ist = ist_.lock();
  if (!ist) {
    return AVERROR_STREAM_NOT_FOUND;
  }
  if (!ist->codec_ctx_) {
    av_log(this, AV_LOG_ERROR, "No decoder, filtering impossible\n");
    return AVERROR_DECODER_NOT_FOUND;
  }
  enum AVMediaType type =
      avfilter_pad_get_type(in->filter_ctx->input_pads, in->pad_idx);
  switch (type) {
    case AVMEDIA_TYPE_VIDEO:
      return configure_input_video_filter(in);
    case AVMEDIA_TYPE_AUDIO:
      return configure_input_audio_filter(in);
    default:
      av_log(
          this, AV_LOG_ERROR,
          "unsupported media_type %s, Only video and audio filters supported "
          "currently.\n",
          av_get_media_type_string(type));
      return AVERROR_UNKNOWN;
  }
}

int InputFilter::configure_input_video_filter(AVFilterInOut *in) {
  auto const &ist = ist_.lock();
  if (!ist) {
    return AVERROR_STREAM_NOT_FOUND;
  }

  const auto &st = ist->av_stream();
  if (!st) {
    return AVERROR_STREAM_NOT_FOUND;
  }

  AVFilterContext *last_filter = nullptr;
  int ret = 0;
  if (ist->codec_ctx_->codec_type == AVMEDIA_TYPE_AUDIO) {
    av_log(this, AV_LOG_ERROR, "Cannot connect video filter to audio input\n");
    ret = AVERROR(EINVAL);
    return ret;
  }

  AVRational fr = ist->codec_ctx_->framerate;
  if (!fr.num) {
    // fr = av_guess_frame_rate(ist->fmt_ctx_.lock(), st, nullptr);
  }
  AVRational sar = sample_aspect_ratio_;
  if (!sar.den) {
    sar = (AVRational){0, 1};
  }

  AVBPrint args;
  SCOPE_EXIT { av_bprint_finalize(&args, nullptr); };

  AVRational tb = ist->codec_ctx_->time_base;
  av_bprint_init(&args, 0, AV_BPRINT_SIZE_AUTOMATIC);
  av_bprintf(&args,
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             width_, height_, format_, tb.num, tb.den, sar.num, sar.den);
  if (fr.num && fr.den) {
    av_bprintf(&args, ":frame_rate=%d/%d", fr.num, fr.den);
  }

  char name[255];
  snprintf(name, sizeof(name), "buffer_in_%d_%d", ist->file_index_,
           ist->stream_index_);

  const AVFilter *buffer_filt = avfilter_get_by_name("buffer");
  last_filter = in->filter_ctx;

#if 0
  auto const &graph = graph_.lock();
  if (!graph) {
    return AVERROR_FILTER_NOT_FOUND;
  }
#endif

  // ret = avfilter_graph_create_filter(&last_filter, buffer_filt, name,
  // args.str,
  //                                   nullptr, graph->filter_graph_.get());
  ret = avfilter_graph_create_filter(&last_filter, buffer_filt, name, args.str,
                                     nullptr, last_filter->graph);
  if (ret < 0) {
    av_log(this, AV_LOG_ERROR, "Cannot create filter %s: %s\n", name,
           av_err2str(ret));
    return ret;
  }
  filter_ = last_filter;

  const AVPixFmtDescriptor *desc =
      av_pix_fmt_desc_get(static_cast<AVPixelFormat>(format_));
  av_assert0(desc);

  int pad_idx = 0;
  // TODO: insert hwaccel enabled filters like transpose_vaapi into the graph
  if (ist->autorotate_ && !(desc->flags & AV_PIX_FMT_FLAG_HWACCEL)) {
    int32_t *displaymatrix = display_matrix_;
    if (!displaymatrix) {
      displaymatrix = (int32_t *)av_stream_get_side_data(
          st, AV_PKT_DATA_DISPLAYMATRIX, nullptr);
    }

    double theta = FilterGraph::get_rotation(displaymatrix);
    snprintf(name, sizeof(name), "in-rotation_%d_%d", ist->file_index_,
             ist->stream_index_);
    if (fabs(theta - 90) < 1.0) {
      ret = FilterGraph::insert_filter(
          &last_filter, &pad_idx, "transpose",
          displaymatrix && displaymatrix[3] > 0 ? "cclock_flip" : "clock");
    } else if (fabs(theta - 180) < 1.0) {
      if (displaymatrix && displaymatrix[0] < 0) {
        ret = FilterGraph::insert_filter(&last_filter, &pad_idx, "hflip",
                                         nullptr);
        if (ret < 0) {
          return ret;
        }
      }
      if (displaymatrix && displaymatrix[4] < 0) {
        ret = FilterGraph::insert_filter(&last_filter, &pad_idx, "vflip",
                                         nullptr);
      }
    } else if (fabs(theta - 270) < 1.0) {
      ret = FilterGraph::insert_filter(
          &last_filter, &pad_idx, "transpose",
          displaymatrix && displaymatrix[3] < 0 ? "clock_flip" : "cclock");
    } else if (fabs(theta) > 1.0) {
      char rotate_buf[64];
      snprintf(rotate_buf, sizeof(rotate_buf), "%f*PI/180", theta);
      ret = FilterGraph::insert_filter(&last_filter, &pad_idx, "rotate",
                                       rotate_buf);
    } else if (fabs(theta) < 1.0) {
      if (displaymatrix && displaymatrix[4] < 0) {
        ret = FilterGraph::insert_filter(&last_filter, &pad_idx, "vflip",
                                         nullptr);
      }
    }
  }

  if ((ret = avfilter_link(last_filter, pad_idx, in->filter_ctx, in->pad_idx)) <
      0) {
    return ret;
  }

  return 0;
}

int InputFilter::configure_input_audio_filter(AVFilterInOut *in) {
  auto const &ist = ist_.lock();
  if (!ist) {
    return AVERROR_STREAM_NOT_FOUND;
  }
  const auto &st = ist->av_stream();
  if (!st) {
    return AVERROR_STREAM_NOT_FOUND;
  }

  AVFilterContext *last_filter;
  int ret = 0;
  if (ist->codec_ctx_->codec_type != AVMEDIA_TYPE_AUDIO) {
    av_log(this, AV_LOG_ERROR,
           "Cannot connect audio filter to non audio input\n");
    ret = AVERROR(EINVAL);
    return ret;
  }

  AVBPrint args;
  SCOPE_EXIT { av_bprint_finalize(&args, nullptr); };

  av_bprint_init(&args, 0, AV_BPRINT_SIZE_AUTOMATIC);
  av_bprintf(&args, "time_base=%d/%d:sample_rate=%d:sample_fmt=%s",
             ist->codec_ctx_->time_base.num, ist->codec_ctx_->time_base.den,
             sample_rate_,
             av_get_sample_fmt_name(static_cast<AVSampleFormat>(format_)));

  if (av_channel_layout_check(&ch_layout_) &&
      ch_layout_.order != AV_CHANNEL_ORDER_UNSPEC) {
    av_bprintf(&args, ":channel_layout=");
    av_channel_layout_describe_bprint(&ch_layout_, &args);
  } else {
    av_bprintf(&args, ":channels=%d", ch_layout_.nb_channels);
  }
  last_filter = in->filter_ctx;

  char name[255];
  snprintf(name, sizeof(name), "abuffer_in_%d_%d", ist->file_index_,
           ist->stream_index_);
  const AVFilter *abuffer_filt = avfilter_get_by_name("abuffer");

#if 0
  auto const &graph = graph_.lock();
  if (!graph) {
    return AVERROR_FILTER_NOT_FOUND;
  }
#endif

  // ret = avfilter_graph_create_filter(&last_filter, abuffer_filt, name,
  // args.str,
  //                                   nullptr, graph->filter_graph_.get());
  ret = avfilter_graph_create_filter(&last_filter, abuffer_filt, name, args.str,
                                     nullptr, last_filter->graph);
  if (ret < 0) {
    av_log(this, AV_LOG_ERROR, "Cannot create filter %s: %s\n", name,
           av_err2str(ret));
    return ret;
  }
  filter_ = last_filter;

  if ((ret = avfilter_link(last_filter, 0, in->filter_ctx, in->pad_idx)) < 0) {
    return ret;
  }

  return 0;
}

int InputFilter::ifilter_send_eof(int64_t pts) {
  int ret = 0;
  eof_ = true;

  if (filter_) {
    ret = av_buffersrc_close(filter_, pts, AV_BUFFERSRC_FLAG_PUSH);
    if (ret < 0) {
      return ret;
    }
  } else {
    auto const &ist = ist_.lock();
    if (!ist) {
      return AVERROR_STREAM_NOT_FOUND;
    }
    const auto &st = ist->av_stream();
    if (!st) {
      return AVERROR_STREAM_NOT_FOUND;
    }
    // the filtergraph was never configured
    if (format_ < 0) {
      ret = ifilter_parameters_from_codecpar(st->codecpar);
      if (ret < 0) {
        return ret;
      }
    }
    if (!ifilter_has_input_format()) {
      av_log(this, AV_LOG_ERROR,
             "Cannot determine format of input stream %d:%d after EOF\n",
             ist->file_index_, st->index);
      return AVERROR_INVALIDDATA;
    }
  }

  return 0;
}

int InputFilter::ifilter_parameters_from_codecpar(
    const AVCodecParameters *par) {
  // We never got any input. Set a fake format, which will
  // come from libavformat.
  format_ = par->format;
  sample_rate_ = par->sample_rate;
  width_ = par->width;
  height_ = par->height;
  sample_aspect_ratio_ = par->sample_aspect_ratio;
  int ret = av_channel_layout_copy(&ch_layout_, &par->ch_layout);
  if (ret < 0) {
    return ret;
  }
  return 0;
}

}  // namespace cv
}  // namespace kingfisher
