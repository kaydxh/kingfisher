#include "input_stream.h"

#include "ffmpeg_hw.h"

namespace kingfisher {
namespace cv {

InputStream::InputStream(std::weak_ptr<AVFormatContext> ifmt_ctx,
                         int file_index, unsigned int stream_index)
    : Stream(std::move(ifmt_ctx), file_index, stream_index) {}

InputStream::~InputStream() { av_dict_free(&decoder_opts_); }

int InputStream::init_input_stream() {
  if (decoding_needed_) {
    // const AVCodec *codec = codec_;
    if (!dec_) {
      return AVERROR(EINVAL);
    }

    // 设置时间基准，对于 cuvid 等硬件解码器很重要
    codec_ctx_->pkt_timebase = st_->time_base;

    if (!av_dict_get(decoder_opts_, "threads", nullptr, 0)) {
      av_dict_set(&decoder_opts_, "threads", "auto", 0);
    }
    /* Attached pics are sparse, therefore we would not want to delay their
     * decoding till EOF. */
    if (st_->disposition & AV_DISPOSITION_ATTACHED_PIC) {
      av_dict_set(&decoder_opts_, "threads", "1", 0);
    }

    // 为 CUDA 解码器设置硬件加速选项
    // ffmpeg -h decoder=h264_cuvid 可查看支持的选项
    bool using_cuda = is_prefer_gpu(gpu_id_) && is_cuda_decoder(dec_->name);
    if (using_cuda) {
      // cuvid_dec.c - 设置 GPU ID
      av_dict_set_int(&decoder_opts_, "gpu", gpu_id_, 0);
      // 设置去隔行模式: adaptive
      av_dict_set(&decoder_opts_, "deint", "adaptive", 0);
      // 去隔行时不需要复制第二场，避免重复帧
      av_dict_set_int(&decoder_opts_, "drop_second_field", 1, 0);

      av_log(nullptr, AV_LOG_INFO,
             "Setting GPU ID %" PRId64 " for CUDA decoder [%s] stream #%d:%d\n",
             gpu_id_, dec_->name, file_index_, stream_index_);
    }

    int ret = avcodec_open2(codec_ctx_.get(), dec_, &decoder_opts_);
    if (ret < 0) {
      // 如果 CUDA 解码器初始化失败，尝试回退到软件解码器
      if (using_cuda && auto_switch_to_soft_codec_) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf));
        av_log(nullptr, AV_LOG_WARNING,
               "CUDA decoder [%s] failed for stream #%d:%d, falling back to "
               "software decoder: %s\n",
               dec_->name, file_index_, stream_index_, errbuf);

        // 查找软件解码器
        const AVCodec *sw_dec = avcodec_find_decoder(st_->codecpar->codec_id);
        if (sw_dec) {
          // 重新分配 codec context - 先释放旧的，再创建新的
          codec_ctx_.reset();
          codec_ctx_ = std::shared_ptr<AVCodecContext>(
              avcodec_alloc_context3(sw_dec),
              [](AVCodecContext *ctx) { avcodec_free_context(&ctx); });
          if (!codec_ctx_) {
            return AVERROR(ENOMEM);
          }

          // 复制参数
          ret = avcodec_parameters_to_context(codec_ctx_.get(), st_->codecpar);
          if (ret < 0) {
            return ret;
          }

          codec_ctx_->pkt_timebase = st_->time_base;
          dec_ = sw_dec;

          // 清除 CUDA 相关选项
          av_dict_set(&decoder_opts_, "gpu", nullptr, 0);
          av_dict_set(&decoder_opts_, "deint", nullptr, 0);
          av_dict_set(&decoder_opts_, "drop_second_field", nullptr, 0);

          // 重新尝试打开解码器
          ret = avcodec_open2(codec_ctx_.get(), dec_, &decoder_opts_);
          if (ret >= 0) {
            av_log(nullptr, AV_LOG_INFO,
                   "Successfully switched to software decoder [%s] for stream "
                   "#%d:%d\n",
                   dec_->name, file_index_, stream_index_);
            return 0;
          }
        }
      }
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
