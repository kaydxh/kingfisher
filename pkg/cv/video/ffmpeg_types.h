#ifndef KINGFISHER_PKG_CV_VIDEO_FFMPEG_TYPES_H_
#define KINGFISHER_PKG_CV_VIDEO_FFMPEG_TYPES_H_

#include <memory>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/codec_par.h"
#include "libavcodec/packet.h"
#include "libavutil/avutil.h"
#include "libavutil/frame.h"
#include "libavutil/rational.h"
};

namespace kingfisher {
namespace cv {

struct Frame {
  std::shared_ptr<AVPacket> packet;
  std::shared_ptr<AVFrame> frame;
  int64_t frame_number = 0;
  AVRational time_base = AV_TIME_BASE_Q;
  int64_t pts = AV_NOPTS_VALUE;
  AVMediaType codec_type = AVMEDIA_TYPE_UNKNOWN;
  AVCodecID codec_id = AV_CODEC_ID_NONE;
};

}  // namespace cv
}  // namespace kingfisher

#endif
