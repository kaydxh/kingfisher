#include "stream.h"

#include <memory>

#include "ffmpeg_utils.h"

extern "C" {
#include "libavfilter/avfilter.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
}

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

/* May return NULL (no pixel format found), a static string or a string
 * backed by the bprint. Nothing has been written to the AVBPrint in case
 * NULL is returned. The AVBPrint provided should be clean. */
std::string Stream::choose_pix_fmts(AVFilterGraph *graph) const {
  const AVDictionaryEntry *strict_dict =
      av_dict_get(codec_opts_, "strict", nullptr, 0);
  if (strict_dict) {
    av_opt_set(codec_ctx_.get(), "strict", strict_dict->value, 0);
  }

  if (keep_pix_fmt_) {
    avfilter_graph_set_auto_convert(graph, AVFILTER_AUTO_CONVERT_NONE);
    if (codec_ctx_->pix_fmt == AV_PIX_FMT_NONE) {
      return "";
    }
    return av_get_pix_fmt_name(static_cast<AVPixelFormat>(codec_ctx_->pix_fmt));
  }
  const auto &codec = codec_ctx_->codec;
  if (codec_ctx_->pix_fmt != AV_PIX_FMT_NONE) {
    return av_get_pix_fmt_name(choose_pixel_fmt(av_stream(), codec_ctx_.get(),
                                                codec, codec_ctx_->pix_fmt));
  }
  if (codec && codec->pix_fmts) {
    const enum AVPixelFormat *p;

    p = codec->pix_fmts;
    if (codec_ctx_->strict_std_compliance > FF_COMPLIANCE_UNOFFICIAL) {
      p = get_compliance_normal_pix_fmts(codec, p);
    }

    std::string fmt;
    for (; *p != AV_PIX_FMT_NONE; p++) {
      const char *name = av_get_pix_fmt_name(*p);
      fmt += name;
      fmt += (p[1] == AV_PIX_FMT_NONE ? '\0' : '|');
    }
    return fmt;
  }
  return "";
}

}  // namespace cv
}  // namespace kingfisher
