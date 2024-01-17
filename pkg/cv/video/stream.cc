#include "stream.h"

namespace kingfisher {
namespace cv {

Stream::Stream(std::weak_ptr<AVFormatContext> fmtCtx, int file_index,
               unsigned int stream_index)
    : fmt_ctx_(fmtCtx), file_index_(file_index), stream_index_(stream_index) {}

Stream::~Stream(){};

AVStream *Stream::av_stream() const {
  const auto &ifmt = fmt_ctx_.lock();
  if (!ifmt) {
    return nullptr;
  }
  if (stream_index_ < ifmt->nb_streams) {
    return ifmt->streams[stream_index_];
  }
  return nullptr;
}

}  // namespace cv
}  // namespace kingfisher
