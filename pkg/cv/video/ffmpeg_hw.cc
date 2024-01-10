#include "ffmpeg_hw.h"

namespace kingfisher {
namespace cv {

HWDevicer::HWDevicer() {}
HWDevicer::~HWDevicer() {}

std::shared_ptr<HWDevice> HWDevicer::HWDeviceGetByType(AVHWDeviceType type) {
  std::shared_ptr<HWDevice> found;
  for (size_t i = 0; i < hw_devices_.size(); i++) {
    if (hw_devices_[i]->type == type) {
      if (found) {
        return nullptr;
      }
      found = hw_devices_[i];
    }
  }
  return found;
}

}  // namespace cv
}  // namespace kingfisher
