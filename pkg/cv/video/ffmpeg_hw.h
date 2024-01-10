#ifndef KINGFISHER_PKG_CV_VIDEO_FFMPEG_HW_H_
#define KINGFISHER_PKG_CV_VIDEO_FFMPEG_HW_H_

#include <memory>
#include <vector>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

namespace kingfisher {
namespace cv {

struct HWDevice {
  std::string name;
  enum AVHWDeviceType type;
  AVBufferRef* device_ref = nullptr;
};

class HWDevicer {
 public:
  HWDevicer();
  ~HWDevicer();

  std::shared_ptr<HWDevice> HWDeviceGetByType(AVHWDeviceType type);

 private:
  std::vector<std::shared_ptr<HWDevice>> hw_devices_;
};
}  // namespace cv
}  // namespace kingfisher

#endif
