#include "ffmpeg_filter.h"

#include <iostream>

#include "core/scope_guard.h"
#include "ffmpeg_error.h"
#include "input_filter.h"
#include "output_filter.h"
#include "stream.h"

extern "C" {
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avstring.h"
#include "libavutil/display.h"
#include "libavutil/opt.h"

static const AVClass filter_graph_class = {
    .class_name = "Filter Graph",
    .item_name = av_default_item_name,
    .option = nullptr,
    .version = LIBAVUTIL_VERSION_INT,
};
};

namespace kingfisher {
namespace cv {

FilterGraph::FilterGraph(std::weak_ptr<Stream> stream,
                         const std::string &graph_desc)
    : av_class_(&filter_graph_class),
      stream_(stream),
      graph_desc_(graph_desc) {}

FilterGraph::~FilterGraph() {}

void FilterGraph::cleanup_filtergraph() {
  inputs_.clear();
  outputs_.clear();
  // AVFilterGraph *fg = filter_graph_.get();
  // avfilter_graph_free(&fg);
}

int FilterGraph::init_simple_filtergraph() {
  inputs_.emplace_back(
      std::make_shared<InputFilter>(shared_from_this(), stream_));
  outputs_.emplace_back(
      std::make_shared<OutputFilter>(shared_from_this(), stream_));
  return 0;
}

static int filter_is_buffersrc(const AVFilterContext *f) {
  return f->nb_inputs == 0 && (!strcmp(f->filter->name, "buffersrc") ||
                               !strcmp(f->filter->name, "abuffersrc"));
}

static int graph_is_meta(AVFilterGraph *graph) {
  for (unsigned i = 0; i < graph->nb_filters; i++) {
    const AVFilterContext *f = graph->filters[i];

    /* in addition to filters flagged as meta, also
     * disregard sinks and buffersources (but not other sources,
     * since they introduce data we are not aware of)
     */
    if (!((f->filter->flags & AVFILTER_FLAG_METADATA_ONLY) ||
          f->nb_outputs == 0 || filter_is_buffersrc(f))) {
      return 0;
    }
  }
  return 1;
}

int FilterGraph::filtergraph_is_simple() { return !graph_desc_.empty(); }

int FilterGraph::configure_filtergraph() {
  int ret = 0;
  filter_graph_ = std::shared_ptr<AVFilterGraph>(
      avfilter_graph_alloc(),
      [](AVFilterGraph *graph) { avfilter_graph_free(&graph); });
  if (!filter_graph_) {
    ret = AVERROR(ENOMEM);
    return ret;
  }

  const AVDictionaryEntry *e = nullptr;
  const std::shared_ptr<Stream> &ost = stream_.lock();
  if (!ost) {
    return AVERROR_STREAM_NOT_FOUND;
  }

  if (filter_nbthreads_) {
    ret = av_opt_set_int(filter_graph_.get(), "threads", filter_nbthreads_, 0);
    if (ret < 0) {
      return ret;
    }
  } else {
    e = av_dict_get(ost->codec_opts_, "threads", nullptr, 0);
    if (e) {
      av_opt_set(filter_graph_.get(), "threads", e->value, 0);
    }
  }

  char args[512] = {0};
  args[0] = 0;
  e = nullptr;
  while ((e = av_dict_get(ost->sws_dict_, "", e, AV_DICT_IGNORE_SUFFIX))) {
    av_strlcatf(args, sizeof(args), "%s=%s:", e->key, e->value);
  }
  if (strlen(args)) {
    args[strlen(args) - 1] = 0;
    filter_graph_->scale_sws_opts = av_strdup(args);
  }

  args[0] = 0;
  e = nullptr;
  while ((e = av_dict_get(ost->swr_opts_, "", e, AV_DICT_IGNORE_SUFFIX))) {
    av_strlcatf(args, sizeof(args), "%s=%s:", e->key, e->value);
  }
  if (strlen(args)) {
    args[strlen(args) - 1] = 0;
  }
  av_opt_set(filter_graph_.get(), "aresample_swr_opts", args, 0);

  AVFilterInOut *inputs = nullptr;
  AVFilterInOut *outputs = nullptr;

  SCOPE_EXIT {
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
  };

  if ((ret = avfilter_graph_parse2(filter_graph_.get(), graph_desc_.c_str(),
                                   &inputs, &outputs)) < 0) {
    return ret;
  }
  // todo
  //
  AVFilterInOut *cur = nullptr;
  int i;
  for (cur = inputs, i = 0; cur; cur = cur->next, i++) {
    if ((ret = inputs_[i]->configure_input_filter(cur)) < 0) {
      return ret;
    }
  }
  for (cur = outputs, i = 0; cur; cur = cur->next, i++) {
    if ((ret = outputs_[i]->configure_output_filter(cur)) < 0) {
      return ret;
    }
  }

  if (!auto_conversion_filters_) {
    avfilter_graph_set_auto_convert(filter_graph_.get(),
                                    AVFILTER_AUTO_CONVERT_NONE);
  }
  if ((ret = avfilter_graph_config(filter_graph_.get(), nullptr)) < 0) {
    return ret;
  }

  is_meta_ = graph_is_meta(filter_graph_.get());

  /* limit the lists of allowed formats to the ones selected, to
   * make sure they stay the same if the filtergraph is reconfigured later */
  for (auto &ofilter : outputs_) {
    const auto &sink = ofilter->filter_;

    ofilter->format_ = av_buffersink_get_format(sink);

    ofilter->width_ = av_buffersink_get_w(sink);
    ofilter->height_ = av_buffersink_get_h(sink);

    ofilter->sample_rate_ = av_buffersink_get_sample_rate(sink);
    av_channel_layout_uninit(&ofilter->ch_layout_);
    ret = av_buffersink_get_ch_layout(sink, &ofilter->ch_layout_);
    if (ret < 0) {
      return ret;
    }
  }

  reconfiguration_ = true;

  for (auto &ofilter : outputs_) {
    const auto &ost = ofilter->ost_.lock();
    if (ost) {
      const auto &st = ost->av_stream();
      if (!st) {
        return AVERROR_STREAM_NOT_FOUND;
      }
      if (!ost->codec_ctx_->codec) {
        /* identical to the same check in ffmpeg.c, needed because
           complex filter graphs are initialized earlier */
        av_log((void *)(&(this->av_class_)), AV_LOG_ERROR,
               "Encoder (codec %s) not found for output stream #%d:%d\n",
               avcodec_get_name(st->codecpar->codec_id), ost->file_index_,
               ost->stream_index_);
        ret = AVERROR(EINVAL);
        return ret;
      }
      if (ost->codec_ctx_->codec->type == AVMEDIA_TYPE_AUDIO &&
          !(ost->codec_ctx_->codec->capabilities &
            AV_CODEC_CAP_VARIABLE_FRAME_SIZE)) {
        av_buffersink_set_frame_size(ofilter->filter_,
                                     ost->codec_ctx_->frame_size);
      }
    }
  }

  for (auto &ifilter : inputs_) {
    AVFrame *tmp;
    while (av_fifo_read(ifilter->frame_queue_.get(), &tmp, 1) >= 0) {
      ret = av_buffersrc_add_frame(ifilter->filter_, tmp);
      av_frame_free(&tmp);
      if (ret < 0) {
        return ret;
      }
    }
  }

  /* send the EOFs for the finished inputs */
  for (auto &ifilter : inputs_) {
    if (ifilter->eof_) {
      ret = av_buffersrc_add_frame(ifilter->filter_, nullptr);
      if (ret < 0) {
        return ret;
      }
    }
  }

  return 0;
}

int FilterGraph::reap_filters(
    std::vector<std::shared_ptr<AVFrame>> &filtered_frames,
    bool need_filtered_frames) {
  filtered_frames.clear();
  int ret = 0;
  for (size_t i = 0; i < outputs_.size(); i++) {
    ret = outputs_[i]->reap_filters(filtered_frames, need_filtered_frames);
    if (ret == AVERROR_EOF) {
      ret = 0; /* ignore */
    }
    if (ret < 0) {
      av_log((void *)(&(this->av_class_)), AV_LOG_ERROR,
             "Failed to inject frame into filter network: %s\n",
             av_err2str(ret));
      break;
    }
  }

  return 0;
}

int FilterGraph::insert_filter(AVFilterContext **last_filter, int *pad_idx,
                               const char *filter_name, const char *args) {
  AVFilterGraph *graph = (*last_filter)->graph;
  int ret = 0;
  const AVFilter *filter = avfilter_get_by_name(filter_name);
  if (!filter) {
    av_log(nullptr, AV_LOG_ERROR, "filtering %s element not found\n",
           filter_name);
    return AVERROR_UNKNOWN;
  }

  AVFilterContext *ctx;
  ret = avfilter_graph_create_filter(&ctx, filter, filter_name, args, nullptr,
                                     graph);
  if (ret < 0) {
    av_log(nullptr, AV_LOG_ERROR, "Cannot create filter %s: %s\n", filter_name,
           av_err2str(ret));
    return ret;
  }

  ret = avfilter_link(*last_filter, *pad_idx, ctx, 0);
  if (ret < 0) {
    av_log(nullptr, AV_LOG_ERROR, "Cannot link filter %s: %s\n", filter_name,
           av_err2str(ret));
    return ret;
  }
  *last_filter = ctx;
  *pad_idx = 0;
  return 0;
}

double FilterGraph::get_rotation(int32_t *displaymatrix) {
  double theta = 0;
  if (displaymatrix) {
    theta = -round(av_display_rotation_get((int32_t *)displaymatrix));
  }

  theta -= 360 * floor(theta / 360 + 0.9 / 360);

  if (fabs(theta - 90 * round(theta / 90)) > 2) {
    av_log(nullptr, AV_LOG_WARNING,
           "Odd rotation angle.\n"
           "If you want to help, upload a sample "
           "of this file to https://streams.videolan.org/upload/ "
           "and contact the ffmpeg-devel mailing list. "
           "(ffmpeg-devel@ffmpeg.org)\n");
  }
  return theta;
}

int FilterGraph::send_frame_to_filters(
    const std::shared_ptr<AVFrame> &decoded_frame) {
  int ret = 0;
  for (unsigned int i = 0; i < inputs_.size(); i++) {
    ret = inputs_[i]->ifilter_send_frame(decoded_frame, i < inputs_.size() - 1);
    if (ret == AVERROR_EOF) {
      ret = 0; /* ignore */
    }
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR,
             "Failed to inject frame into filter network: %s\n",
             av_err2str(ret));
      break;
    }
  }
  return ret;
}

int FilterGraph::send_filter_eof(int64_t pts) {
  int ret = 0;
  for (unsigned int i = 0; i < inputs_.size(); i++) {
    ret = inputs_[i]->ifilter_send_eof(pts);
    if (ret == AVERROR_EOF) {
      ret = 0; /* ignore */
    }
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR,
             "Failed to inject frame into filter network: %s\n",
             av_err2str(ret));
      break;
    }
  }
  return ret;
}

}  // namespace cv
}  // namespace kingfisher
