#ifndef KINGFISHER_PKG_CV_VIDEO_FFMPEG_HW_H_
#define KINGFISHER_PKG_CV_VIDEO_FFMPEG_HW_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

namespace kingfisher {
namespace cv {

// 检查是否偏好使用 GPU（gpu_id >= 0 表示使用 GPU）
bool is_prefer_gpu(int64_t gpu_id);

// 根据 codec_id 获取对应的 CUDA 解码器名称
// 返回空字符串表示不支持 CUDA 解码
const char* get_cuda_decoder_name(AVCodecID codec_id);

// 根据 codec_id 获取对应的 NVENC 编码器名称
// 返回空字符串表示不支持 NVENC 编码
const char* get_nvenc_encoder_name(AVCodecID codec_id);

// 检查指定的编解码器是否是 CUDA/NVENC 硬件编解码器
bool is_cuda_decoder(const char* codec_name);
bool is_nvenc_encoder(const char* codec_name);

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
