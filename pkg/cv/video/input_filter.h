#ifndef KINGFISHER_PKG_CV_VIDEO_INPUT_FILTER_H_
#define KINGFISHER_PKG_CV_VIDEO_INPUT_FILTER_H_

#include <memory>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
#include "libavutil/fifo.h"
};

namespace kingfisher {
namespace cv {

class Stream;
class FilterGraph;

class InputFilter {
 public:
  InputFilter(const std::shared_ptr<FilterGraph> &fg,
              const std::weak_ptr<Stream> &ist);
  ~InputFilter();

  int ifilter_send_frame(const std::shared_ptr<AVFrame> &frame,
                         int keep_reference);

  int ifilter_parameters_from_frame(const std::shared_ptr<AVFrame> &frame);
  bool ifilter_has_input_format() const;

  int configure_input_filter(AVFilterInOut *in);
  int configure_input_video_filter(AVFilterInOut *in);
  int configure_input_audio_filter(AVFilterInOut *in);
  int ifilter_send_eof(int64_t pts);
  int ifilter_parameters_from_codecpar(const AVCodecParameters *par);

 public:
  const AVClass *av_class_ = nullptr;
  std::shared_ptr<AVFilterContext> filter_;
  // 避免相互引用
  std::weak_ptr<FilterGraph> graph_;
  std::weak_ptr<Stream> ist_;
  std::shared_ptr<AVFifo> frame_queue_;
  std::string name_;
  enum AVMediaType type_;  //  AVMEDIA_TYPE_SUBTITLE for sub2video
                           //
  int format_ = -1;
  int width_ = 0;
  int height_ = 0;
  AVRational sample_aspect_ratio_;

  int sample_rate_ = 0;

  AVChannelLayout ch_layout_;

  AVBufferRef *hw_frames_ctx_ = nullptr;
  int32_t *display_matrix_ = nullptr;

  bool eof_ = false;
};

}  // namespace cv
}  // namespace kingfisher

#endif
