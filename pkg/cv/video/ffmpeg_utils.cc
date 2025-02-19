#include "ffmpeg_utils.h"

#include <strings/strings.h>
extern "C" {
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
static const AVClass ffmpeg_util_class = {
    .class_name = "FFmpeg Util",
    .item_name = av_default_item_name,
    .option = nullptr,
    .version = LIBAVUTIL_VERSION_INT,
};
}

namespace kingfisher {
namespace cv {

int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec) {
  int ret = avformat_match_stream_specifier(s, st, spec);
  if (ret < 0) {
    av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
  }
  return ret;
}

// 多个匹配时,告警提示
#if 0
std::string warn_mulitiple_opt_usage(void *avcl, AVStream *st,
                                     const std::string &option_name,
                                     const std::vector<std::string> &vals) {
  std::string s;
  s = strings::Join(vals, ",");
  std::string warning_msg = "Multiple " + s + " options specified for stream " +
                            std::to_string(st->index) +
                            ", only the last option '" + vals.back() +
                            "' will be used.\n";

  av_log(avcl, AV_LOG_WARNING, "%s", warning_msg.c_str());
  return warning_msg;
}
#endif

/// https://sourcegraph.com/github.com/FFmpeg/FFmpeg@ae14d9c06bcddc5d4c14de02e049f489ddbf73a4/-/blob/fftools/ffmpeg_opt.c#:~:text=static%20const%20AVCodec%20*find_codec_or_die(const%20char%20*name%2C%20enum%20AVMediaType%20type%2C%20int%20encoder)
int find_codec(void *avcl, const std::string &name, enum AVMediaType type,
               bool encoder, const AVCodec *&codec, bool recast_media) {
  const AVCodecDescriptor *desc;
  const char *codec_string = encoder ? "encoder" : "decoder";
  codec = encoder ? avcodec_find_encoder_by_name(name.c_str())
                  : avcodec_find_decoder_by_name(name.c_str());

  if (!codec && (desc = avcodec_descriptor_get_by_name(name.c_str()))) {
    codec = encoder ? avcodec_find_encoder(desc->id)
                    : avcodec_find_decoder(desc->id);
    if (codec) {
      av_log(avcl, AV_LOG_VERBOSE, "Matched %s '%s' for codec '%s'.\n",
             codec_string, codec->name, desc->name);
    }
  }

  if (!codec) {
    // av_log(avcl, AV_LOG_ERROR, "Unknown %s '%s'\n", codec_string,
    // name.c_str());
    return AVERROR(EINVAL);
  }
  if (codec->type != type && !recast_media) {
    av_log(avcl, AV_LOG_ERROR, "Invalid %s type '%s'\n", codec_string,
           name.c_str());
    return AVERROR(EINVAL);
  }
  return 0;
}

AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                AVFormatContext *s, AVStream *st,
                                const AVCodec *codec) {
  AVDictionary *ret = nullptr;
  const AVDictionaryEntry *t = nullptr;
  int flags =
      s->oformat ? AV_OPT_FLAG_ENCODING_PARAM : AV_OPT_FLAG_DECODING_PARAM;
  char prefix = 0;
  const AVClass *cc = avcodec_get_class();

  if (!codec) {
    codec = s->oformat ? avcodec_find_encoder(codec_id)
                       : avcodec_find_decoder(codec_id);
  }

  switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
      prefix = 'v';
      flags |= AV_OPT_FLAG_VIDEO_PARAM;
      break;
    case AVMEDIA_TYPE_AUDIO:
      prefix = 'a';
      flags |= AV_OPT_FLAG_AUDIO_PARAM;
      break;
    case AVMEDIA_TYPE_SUBTITLE:
      prefix = 's';
      flags |= AV_OPT_FLAG_SUBTITLE_PARAM;
      break;

    default:
      return nullptr;
  }

  while ((t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX))) {
    const AVClass *priv_class;
    char *p = strchr(t->key, ':');

    /* check stream specification in opt name */
    if (p) {
      switch (check_stream_specifier(s, st, p + 1)) {
        case 1:
          *p = 0;
          break;
        case 0:
          continue;
        default:
          return nullptr;
      }
    }

    if (av_opt_find(&cc, t->key, nullptr, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
        !codec ||
        ((priv_class = codec->priv_class) &&
         av_opt_find(&priv_class, t->key, nullptr, flags,
                     AV_OPT_SEARCH_FAKE_OBJ))) {
      av_dict_set(&ret, t->key, t->value, 0);
    } else if (t->key[0] == prefix &&
               av_opt_find(&cc, t->key + 1, nullptr, flags,
                           AV_OPT_SEARCH_FAKE_OBJ)) {
      av_dict_set(&ret, t->key + 1, t->value, 0);
    }

    if (p) {
      *p = ':';
    }
  }
  return ret;
}

// FIXME: YUV420P etc. are actually supported with full color range,
// yet the latter information isn't available here.
const AVPixelFormat *get_compliance_normal_pix_fmts(
    const AVCodec *codec, const AVPixelFormat default_formats[]) {
  static const enum AVPixelFormat mjpeg_formats[] = {
      AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUVJ422P, AV_PIX_FMT_YUVJ444P,
      AV_PIX_FMT_NONE};

  if (!strcmp(codec->name, "mjpeg")) {
    return mjpeg_formats;
  } else {
    return default_formats;
  }
}

AVPixelFormat choose_pixel_fmt(const AVStream *st, AVCodecContext *enc_ctx,
                               const AVCodec *codec,
                               enum AVPixelFormat target) {
  if (codec && codec->pix_fmts) {
    // FIXME: take null as all pix fmts
    const enum AVPixelFormat *p = codec->pix_fmts;

    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(target);
    // FIXME: This should check for AV_PIX_FMT_FLAG_ALPHA after PAL8 pixel
    // format without alpha is implemented
    int has_alpha = desc ? desc->nb_components % 2 == 0 : 0;
    enum AVPixelFormat best = AV_PIX_FMT_NONE;

    if (enc_ctx->strict_std_compliance > FF_COMPLIANCE_UNOFFICIAL) {
      p = get_compliance_normal_pix_fmts(codec, p);
    }
    for (; *p != AV_PIX_FMT_NONE; p++) {
      best = av_find_best_pix_fmt_of_2(best, *p, target, has_alpha, nullptr);
      if (*p == target) break;
    }
    if (*p == AV_PIX_FMT_NONE) {
      if (target != AV_PIX_FMT_NONE) {
        av_log(nullptr, AV_LOG_WARNING,
               "Incompatible pixel format '%s' for codec '%s', auto-selecting "
               "format '%s'\n",
               av_get_pix_fmt_name(target), codec->name,
               av_get_pix_fmt_name(best));
      }
      return best;
    }
  }
  return target;
}

void av_frame_rescale_ts(AVFrame *pkt, AVRational src_tb, AVRational dst_tb) {
  if (!av_cmp_q(src_tb, dst_tb)) {
    return;
  }
  if (pkt->pts != AV_NOPTS_VALUE) {
    pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
  }
  if (pkt->pkt_dts != AV_NOPTS_VALUE) {
    pkt->pkt_dts = av_rescale_q(pkt->pkt_dts, src_tb, dst_tb);
  }
#if (LIBAVCODEC_VERSION_MAJOR < 60)
  if (pkt->pkt_duration > 0) {
    pkt->pkt_duration = av_rescale_q(pkt->pkt_duration, src_tb, dst_tb);
  }
#else
  if (pkt->duration > 0) {
    pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);
  }
#endif
}

}  // namespace cv
}  // namespace kingfisher
