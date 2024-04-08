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

OutputStream::OutputStream(std::weak_ptr<AVFormatContext> ofmt_ctx,
                           int file_index, unsigned int stream_index)
    : Stream(std::move(ofmt_ctx), file_index, stream_index),
      av_class_(&output_stream_class) {}

OutputStream::~OutputStream() { avcodec_parameters_free(&ref_par_); }

}  // namespace cv
}  // namespace kingfisher

