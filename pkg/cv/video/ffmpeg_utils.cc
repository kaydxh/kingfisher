#include "ffmpeg_utils.h"

#include <strings/strings.h>

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

int match_per_stream_opt(void *avcl, AVDictionary *opts, AVFormatContext *s,
                         AVStream *st, const std::string &option_name,
                         std::vector<std::string> &vals) {
  int matches = 0;

  for (unsigned int i = 0; i < vals.size(); i++) {
    AVDictionaryEntry *entry =
        av_dict_get(opts, (option_name + vals[i]).c_str(), nullptr, 0);
    if (entry) {
      int ret = check_stream_specifier(s, st, entry->value);
      if (ret > 0) {
        matches++;
      } else if (ret < 0) {
        av_log(avcl, AV_LOG_ERROR,
               "Invalid stream specifier for option '%s': %s\n", entry->key,
               entry->value);
        return ret;
      }
    }
  }

  if (matches > 1) {
    warn_mulitiple_opt_usage(avcl, st, option_name, vals);
  }

  return matches;
}

int match_per_stream_opt_one(void *avcl, AVDictionary *opts, AVFormatContext *s,
                             AVStream *st, const std::string &option_name,
                             std::string &val) {
  std::vector<std::string> vals(1);
  int ret = match_per_stream_opt(avcl, opts, s, st, option_name, vals);
  if (ret < 0) {
    return ret;
  }

  if (!vals.empty()) {
    val = vals[0];
  }
  return 0;
}

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
    av_log(avcl, AV_LOG_FATAL, "Unknown %s '%s'\n", codec_string, name.c_str());
    return AVERROR(EINVAL);
  }
  if (codec->type != type && !recast_media) {
    av_log(avcl, AV_LOG_FATAL, "Invalid %s type '%s'\n", codec_string,
           name.c_str());
    return AVERROR(EINVAL);
  }
  return 0;
}

}  // namespace cv
}  // namespace kingfisher
