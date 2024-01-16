#include "input_stream.h"

namespace kingfisher {
namespace cv {

InputStream::InputStream(std::weak_ptr<AVFormatContext> ifmtCtx, int file_index,
                         unsigned int stream_index)
    : Stream(ifmtCtx, file_index, stream_index) {}

InputStream::~InputStream() { av_dict_free(&decoder_opts_); }

int InputStream::init_input_stream() {
  if (decoding_needed_) {
    // const AVCodec *codec = codec_;
    if (!codec_) {
      return AVERROR(EINVAL);
    }

    if (!av_dict_get(decoder_opts_, "threads", nullptr, 0)) {
      av_dict_set(&decoder_opts_, "threads", "auto", 0);
    }
    /* Attached pics are sparse, therefore we would not want to delay their
     * decoding till EOF. */
    if (st_->disposition & AV_DISPOSITION_ATTACHED_PIC) {
      av_dict_set(&decoder_opts_, "threads", "1", 0);
    }

    // todo  hw
    int ret = avcodec_open2(codec_ctx_.get(), codec_.get(), &decoder_opts_);
    if (ret < 0) {
      /*
    snprintf(error, error_len,
             "Error while opening decoder for input stream "
             "#%d:%d : %s",
             ist->file_index, ist->st->index, av_err2str(ret));
             */
      return ret;
    }
    return 0;
  }
  return 0;
}

}  // namespace cv
}  // namespace kingfisher
