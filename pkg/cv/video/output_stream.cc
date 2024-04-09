#include "output_stream.h"

extern "C" {
static const AVClass output_stream_class = {
    .class_name = "Output Stream",
    .item_name = av_default_item_name,
    .option = nullptr,
    .version = LIBAVUTIL_VERSION_INT,
};
}

namespace kingfisher {
namespace cv {

OutputStream::OutputStream(std::shared_ptr<AVFormatContext> ifmt_ctx,
                           std::weak_ptr<AVFormatContext> ofmt_ctx,
                           int file_index, unsigned int stream_index)
    : Stream(std::move(ofmt_ctx), file_index, stream_index),
      av_class_(&output_stream_class),
      ifmt_ctx_(ifmt_ctx) {}

OutputStream::~OutputStream() { avcodec_parameters_free(&ref_par_); }

AVStream *OutputStream::input_av_stream() const {
  if (!ifmt_ctx_) {
    return nullptr;
  }

  // inputstream 和 outputstream 暂时共用一个stream_index
  if (stream_index_ < ifmt_ctx_->nb_streams) {
    return ifmt_ctx_->streams[stream_index_];
  }

  return nullptr;
}

}  // namespace cv
}  // namespace kingfisher

