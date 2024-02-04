#include "ffmpeg_filter.h"

#include "ffmpeg_error.h"
#include "stream.h"

extern "C" {
#include "libavutil/avstring.h"
#include "libavutil/opt.h"
};

namespace kingfisher {
namespace cv {

FilterGraph::FilterGraph() {}
FilterGraph::~FilterGraph() {}

void FilterGraph::cleanup_filtergraph() {
  inputs_.clear();
  outputs_.clear();
  AVFilterGraph *fg = filter_graph_.get();
  avfilter_graph_free(&fg);
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
          f->nb_outputs == 0 || filter_is_buffersrc(f)))
      return 0;
  }
  return 1;
}

int FilterGraph::filtergraph_is_simple() { return !graph_desc_.empty(); }

int FilterGraph::configure_filtergraph() {
  filter_graph_ = std::shared_ptr<AVFilterGraph>(
      avfilter_graph_alloc(),
      [](AVFilterGraph *graph) { avfilter_graph_free(&graph); });

  int ret = 0;
  const AVDictionaryEntry *e = nullptr;

  const std::shared_ptr<Stream> &ost = output_stream_.lock();
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

  if ((ret = avfilter_graph_parse2(filter_graph_.get(), graph_desc_.c_str(),
                                   &inputs, &outputs)) < 0) {
    return ret;
  }

  return 0;
}

int FilterGraph::reap_filters() { return 0; }

}  // namespace cv
}  // namespace kingfisher
