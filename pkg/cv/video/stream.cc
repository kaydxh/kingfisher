#include "stream.h"

#include <memory>

namespace kingfisher {
namespace cv {

Stream::Stream(std::weak_ptr<AVFormatContext> fmt_ctx, AVStream *st,
               int file_index, unsigned int stream_index)
    : fmt_ctx_(std::move(fmt_ctx)),
      st_(st),
      file_index_(file_index),
      stream_index_(stream_index),
      frame_(std::shared_ptr<AVFrame>(
          av_frame_alloc(), [](AVFrame *frame) { av_frame_free(&frame); })),
      pkt_(std::shared_ptr<AVPacket>(
          av_packet_alloc(), [](AVPacket *packet) { av_packet_free(&packet); }))

{}

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
