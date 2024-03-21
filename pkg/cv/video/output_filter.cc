#include "output_filter.h"

#include "ffmpeg_error.h"
#include "ffmpeg_filter.h"
#include "ffmpeg_utils.h"
#include "stream.h"

extern "C" {
#include "libavfilter/buffersink.h"
#include "libavutil/bprint.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
static const AVClass output_filter_class = {
    .class_name = "Output Filter",
    .item_name = av_default_item_name,
    .option = nullptr,
    .version = LIBAVUTIL_VERSION_INT,
};
}

namespace kingfisher {
namespace cv {

OutputFilter::OutputFilter(const std::shared_ptr<FilterGraph> &fg,
                           const std::weak_ptr<Stream> &ost)
    : av_class_(&output_filter_class), graph_(fg), ost_(ost) {}

OutputFilter::~OutputFilter() {}

int OutputFilter::configure_output_filter(AVFilterInOut *out) {
  auto const &ost = ost_.lock();
  if (!ost) {
    return AVERROR_STREAM_NOT_FOUND;
  }
  if (!ost->codec_ctx_) {
    av_log(this, AV_LOG_ERROR, "No decoder, filtering impossible\n");
    return AVERROR_ENCODER_NOT_FOUND;
  }

  enum AVMediaType type =
      avfilter_pad_get_type(out->filter_ctx->input_pads, out->pad_idx);
  switch (type) {
    case AVMEDIA_TYPE_VIDEO:
      return configure_output_video_filter(out);
    case AVMEDIA_TYPE_AUDIO:
      return configure_output_audio_filter(out);
    default:
      av_log(
          this, AV_LOG_ERROR,
          "unsupported media_type %s, Only video and audio filters supported "
          "currently.\n",
          av_get_media_type_string(type));
      return AVERROR_UNKNOWN;
  }
  return 0;
}

int OutputFilter::configure_output_video_filter(AVFilterInOut *out) {
  auto const &ost = ost_.lock();
  if (!ost) {
    return AVERROR_STREAM_NOT_FOUND;
  }

  int ret = 0;
  if (ost->codec_ctx_->codec_type == AVMEDIA_TYPE_AUDIO) {
    av_log(this, AV_LOG_ERROR, "Cannot connect video filter to audio input\n");
    ret = AVERROR(EINVAL);
    return ret;
  }

  auto const &graph = graph_.lock();
  if (!graph) {
    return AVERROR_FILTER_NOT_FOUND;
  }

  AVFilterContext *last_filter = out->filter_ctx;
  char name[255];
  int last_pad_idx = out->pad_idx;

  if ((width_ || height_) && ost->autoscale_) {
    char args[255];
    const AVDictionaryEntry *e = nullptr;

    snprintf(args, sizeof(args), "%d:%d", width_, height_);

    while ((e = av_dict_get(ost->sws_dict_, "", e, AV_DICT_IGNORE_SUFFIX))) {
      av_strlcatf(args, sizeof(args), ":%s=%s", e->key, e->value);
    }

    snprintf(name, sizeof(name), "scaler_out_%d_%d", ost->file_index_,
             ost->stream_index_);

    if ((ret = FilterGraph::insert_filter(&last_filter, &last_pad_idx, "scale",
                                          args)) < 0) {
      av_log(this, AV_LOG_ERROR, "Cannot create filter %s: %s\n", name,
             av_err2str(ret));
      return ret;
    }
  }

  std::string pix_fmts = ost->choose_pix_fmts(last_filter->graph);
  if (!pix_fmts.empty()) {
    snprintf(name, sizeof(name), "format_out_%d_%d", ost->file_index_,
             ost->stream_index_);
    if ((ret = FilterGraph::insert_filter(&last_filter, &last_pad_idx, "format",
                                          pix_fmts.c_str())) < 0) {
      av_log(this, AV_LOG_ERROR, "Cannot create filter %s: %s\n", name,
             av_err2str(ret));
      return ret;
    }
  }

  snprintf(name, sizeof(name), "buffersink_out_%d_%d", ost->file_index_,
           ost->stream_index_);
  if ((ret = FilterGraph::insert_filter(&last_filter, &last_pad_idx,
                                        "buffersink", nullptr)) < 0) {
    av_log(this, AV_LOG_ERROR, "Cannot create filter %s: %s\n", name,
           av_err2str(ret));
    return ret;
  }

  filter_ = last_filter;
  return 0;
}

int OutputFilter::configure_output_audio_filter(AVFilterInOut *in) { return 0; }

}  // namespace cv
}  // namespace kingfisher
