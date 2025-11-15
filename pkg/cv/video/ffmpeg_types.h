#ifndef KINGFISHER_PKG_CV_VIDEO_FFMPEG_TYPES_H_
#define KINGFISHER_PKG_CV_VIDEO_FFMPEG_TYPES_H_

#include <memory>
#include <string>
#include <vector>

#ifdef ENABLE_OPENCV
#include <opencv2/opencv.hpp>
#endif

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/codec_par.h"
#include "libavcodec/packet.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/frame.h"
#include "libavutil/rational.h"
};

namespace kingfisher {
namespace cv {

struct Frame {
  std::shared_ptr<AVPacket> packet;
  std::shared_ptr<AVFrame> frame;
  
#ifdef ENABLE_OPENCV
  ::cv::Mat mat;  // CV_8U类型，解码后的视频帧
  std::vector<std::string> sei_messages;  // for mat, SEI messages
#endif
  
  int64_t frame_number = 0;
  AVRational time_base = AV_TIME_BASE_Q;  // for mat/packet/frame
  int64_t pts = AV_NOPTS_VALUE;  // for mat/packet/frame
  AVMediaType codec_type = AVMEDIA_TYPE_UNKNOWN;
  AVCodecID codec_id = AV_CODEC_ID_NONE;

  bool empty() const {
#ifdef ENABLE_OPENCV
    return mat.empty() && !packet && !frame;
#else
    return !packet && !frame;
#endif
  }
};

struct FormatContext {
  // class StreamContext;

  std::shared_ptr<AVFormatContext> av_format_context;
  std::shared_ptr<AVStream> video_stream;
  std::shared_ptr<AVCodecContext> video_codec_context;
  std::shared_ptr<AVStream> audio_stream;
  std::shared_ptr<AVCodecContext> audio_codec_context;
#if 0
  std::string url;
  AVRational time_base = AV_TIME_BASE_Q;  // AV_TIME_BASE_Q, Internal
                                          // time base represented as
                                          // fractional value
  int64_t start_time =
      AV_NOPTS_VALUE;  // Position of the first frame of the component, in
  // AV_TIME_BASE fractional seconds.
  int64_t duration = AV_NOPTS_VALUE;  // Duration of the stream, in AV_TIME_BASE
                                      // fractional seconds.
  int64_t bit_rate = 0;  // Total stream bitrate in bit/s, 0 if not available.
#endif

#if 0
  std::shared_ptr<StreamContext> video_stream;
  std::shared_ptr<StreamContext> audio_stream;

  struct StreamContext {
    AVStream stream;
    AVCodecContext codec_ctx;
  };
#endif
};

}  // namespace cv
}  // namespace kingfisher

#endif
