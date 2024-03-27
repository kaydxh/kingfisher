#ifndef KINGFISHER_PKG_CV_VIDEO_OUTPUT_FILTER_H_
#define KINGFISHER_PKG_CV_VIDEO_OUTPUT_FILTER_H_

#include <memory>
#include <vector>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
};

namespace kingfisher {
namespace cv {

class Stream;
class FilterGraph;

class OutputFilter {
 public:
  OutputFilter(const std::shared_ptr<FilterGraph> &fg,
               const std::weak_ptr<Stream> &ost);
  ~OutputFilter();

  int configure_output_filter(AVFilterInOut *out);
  int configure_output_video_filter(AVFilterInOut *out);
  int configure_output_audio_filter(AVFilterInOut *out);

  int reap_filters();

 public:
  const AVClass *av_class_ = nullptr;
  AVFilterContext *filter_;
  std::weak_ptr<FilterGraph> graph_;
  std::weak_ptr<Stream> ost_;
  std::string name_;
  enum AVMediaType type_;  //  AVMEDIA_TYPE_SUBTITLE for sub2video
                           //
  /* temporary storage until stream maps are processed */
  std::shared_ptr<AVFilterInOut> out_tmp_;

  /* desired output stream properties */
  int width_;
  int height_;
  AVRational frame_rate_;
  int format_ = -1;
  int sample_rate_ = 0;
  AVChannelLayout ch_layout_;

  // those are only set if no format is specified and the encoder gives us
  // multiple options They point directly to the relevant lists of the encoder.
  const int *formats_ = nullptr;
  const AVChannelLayout *ch_layouts_ = nullptr;
  const int *sample_rates_ = nullptr;

 private:
  std::shared_ptr<AVFrame> filtered_frame_;
  // AVFrame *filtered_frame_ = nullptr;
  std::vector<std::shared_ptr<AVFrame>> filtered_frames_;
  //  std::vector<std::shared_ptr<AVFrame>> filtered_frames_;
};

}  // namespace cv
}  // namespace kingfisher

#endif
