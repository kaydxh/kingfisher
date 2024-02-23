#include "input_stream.h"

namespace kingfisher {
namespace cv {

InputStream::InputStream(std::weak_ptr<AVFormatContext> ifmt_ctx, AVStream *st,
                         int file_index, unsigned int stream_index)
    : Stream(std::move(ifmt_ctx), st, file_index, stream_index) {}

InputStream::~InputStream() { av_dict_free(&decoder_opts_); }

int InputStream::init_input_stream() {
  if (decoding_needed_) {
    // const AVCodec *codec = codec_;
    if (!dec_) {
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
    int ret = avcodec_open2(codec_ctx_.get(), dec_, &decoder_opts_);
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

int InputStream::guess_input_channel_layout() {
  AVCodecContext *dec = codec_ctx_.get();

  if (dec->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC) {
    char layout_name[256] = {0};

    if (dec->ch_layout.nb_channels > guess_layout_max_) {
      return 0;
    }
    av_channel_layout_default(&dec->ch_layout, dec->ch_layout.nb_channels);
    if (dec->ch_layout.order == AV_CHANNEL_ORDER_UNSPEC) {
      return 0;
    }
    av_channel_layout_describe(&dec->ch_layout, layout_name,
                               sizeof(layout_name));
    av_log(nullptr, AV_LOG_WARNING,
           "Guessed Channel Layout for Input Stream "
           "#%d.%d : %s\n",
           file_index_, av_stream()->index, layout_name);
  }
  return 1;
}

}  // namespace cv
}  // namespace kingfisher
