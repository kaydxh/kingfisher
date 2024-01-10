#ifndef KINGFISHER_PKG_CV_VIDEO_INPUT_FILTER_H_
#define KINGFISHER_PKG_CV_VIDEO_INPUT_FILTER_H_

#include <memory>

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

class InputFilter {
 public:
  InputFilter();
  ~InputFilter();

 public:
  std::shared_ptr<AVFilterContext> filter_;
  // 避免相互引用
  std::weak_ptr<Stream> ist_;
  std::weak_ptr<FilterGraph> graph_;
  std::string name_;
  enum AVMediaType type_;  //  AVMEDIA_TYPE_SUBTITLE for sub2video
                           //
  int format_ = -1;
  int width_ = 0;
  int height_ = 0;
  AVRational sample_aspect_ratio_;

  int sample_rate_ = 0;

  AVChannelLayout ch_layout_;

  AVBufferRef *hw_frames_ctx = nullptr;
  int32_t *displaymatrix = nullptr;

  bool eof_ = false;
};

}  // namespace cv
}  // namespace kingfisher

#endif
