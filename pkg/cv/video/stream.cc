#include "stream.h"

namespace kingfisher {
namespace cv {

Stream::Stream(std::weak_ptr<AVFormatContext> fmtCtx, int file_index,
               unsigned int stream_index)
    : fmt_ctx_(fmtCtx), file_index_(file_index), stream_index_(stream_index) {}

Stream::~Stream(){};

}  // namespace cv
}  // namespace kingfisher
