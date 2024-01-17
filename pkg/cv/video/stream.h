#ifndef KINGFISHER_PKG_CV_VIDEO_STREAM_H_
#define KINGFISHER_PKG_CV_VIDEO_STREAM_H_

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
#include <memory>

namespace kingfisher {
namespace cv {

class Stream {
 public:
  Stream(std::weak_ptr<AVFormatContext> fmtCtx, int file_index,
         unsigned int stream_index);

  ~Stream();

  virtual AVStream *av_stream() const;

 public:
  std::weak_ptr<AVFormatContext> fmt_ctx_;
  int file_index_ = 0;
  unsigned int stream_index_ = 0;
  std::shared_ptr<AVStream> st_;
  std::shared_ptr<AVCodecContext> codec_ctx_;  // for decode or encode
  std::shared_ptr<AVCodec> codec_;             // for decode or encode
  std::shared_ptr<AVPacket> pkt_;
  std::shared_ptr<AVFrame> frame_;
};

}  // namespace cv
}  // namespace kingfisher

#endif
