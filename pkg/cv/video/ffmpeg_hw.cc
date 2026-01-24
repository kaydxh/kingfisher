#include "ffmpeg_hw.h"

#include <cstring>

namespace kingfisher {
namespace cv {

// 检查是否偏好使用 GPU
bool is_prefer_gpu(int64_t gpu_id) { return gpu_id >= 0; }

// 根据 codec_id 获取对应的 CUDA 解码器名称
// https://gist.github.com/Brainiarc7/c6164520f082c27ae7bbea9556d4a3ba
// 支持的 CUDA 解码器:
//   h263_cuvid, h264_cuvid, hevc_cuvid, mjpeg_cuvid,
//   mpeg1_cuvid, mpeg2_cuvid, mpeg4_cuvid, vc1_cuvid,
//   vp8_cuvid, vp9_cuvid, av1_cuvid
const char* get_cuda_decoder_name(AVCodecID codec_id) {
  switch (codec_id) {
    case AV_CODEC_ID_H263:
      return "h263_cuvid";
    case AV_CODEC_ID_H264:
      return "h264_cuvid";
    case AV_CODEC_ID_HEVC:
      return "hevc_cuvid";
    case AV_CODEC_ID_MJPEG:
      return "mjpeg_cuvid";
    case AV_CODEC_ID_MPEG1VIDEO:
      return "mpeg1_cuvid";
    case AV_CODEC_ID_MPEG2VIDEO:
      return "mpeg2_cuvid";
    case AV_CODEC_ID_MPEG4:
      return "mpeg4_cuvid";
    case AV_CODEC_ID_VC1:
      return "vc1_cuvid";
    case AV_CODEC_ID_VP8:
      return "vp8_cuvid";
    case AV_CODEC_ID_VP9:
      return "vp9_cuvid";
    case AV_CODEC_ID_AV1:
      return "av1_cuvid";
    default:
      return nullptr;
  }
}

// 根据 codec_id 获取对应的 NVENC 编码器名称
// 支持的 NVENC 编码器:
//   h264_nvenc, hevc_nvenc, av1_nvenc
const char* get_nvenc_encoder_name(AVCodecID codec_id) {
  switch (codec_id) {
    case AV_CODEC_ID_H264:
      return "h264_nvenc";
    case AV_CODEC_ID_HEVC:
      return "hevc_nvenc";
    case AV_CODEC_ID_AV1:
      return "av1_nvenc";
    default:
      return nullptr;
  }
}

// 检查是否是 CUDA 硬件解码器
bool is_cuda_decoder(const char* codec_name) {
  if (!codec_name) {
    return false;
  }
  return strcmp(codec_name, "h263_cuvid") == 0 ||
         strcmp(codec_name, "h264_cuvid") == 0 ||
         strcmp(codec_name, "hevc_cuvid") == 0 ||
         strcmp(codec_name, "mjpeg_cuvid") == 0 ||
         strcmp(codec_name, "mpeg1_cuvid") == 0 ||
         strcmp(codec_name, "mpeg2_cuvid") == 0 ||
         strcmp(codec_name, "mpeg4_cuvid") == 0 ||
         strcmp(codec_name, "vc1_cuvid") == 0 ||
         strcmp(codec_name, "vp8_cuvid") == 0 ||
         strcmp(codec_name, "vp9_cuvid") == 0 ||
         strcmp(codec_name, "av1_cuvid") == 0;
}

// 检查是否是 NVENC 硬件编码器
bool is_nvenc_encoder(const char* codec_name) {
  if (!codec_name) {
    return false;
  }
  return strcmp(codec_name, "h264_nvenc") == 0 ||
         strcmp(codec_name, "hevc_nvenc") == 0 ||
         strcmp(codec_name, "av1_nvenc") == 0 ||
         strcmp(codec_name, "nvenc") == 0 ||
         strcmp(codec_name, "nvenc_h264") == 0 ||
         strcmp(codec_name, "nvenc_hevc") == 0;
}

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
