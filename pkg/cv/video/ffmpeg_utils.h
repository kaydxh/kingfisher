#ifndef KINGFISHER_PKG_CV_VIDEO_FFMPEG_UTILS_H_
#define KINGFISHER_PKG_CV_VIDEO_FFMPEG_UTILS_H_

#include <string>
#include <vector>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
};

namespace kingfisher {
namespace cv {

/**
 * Check if the given stream matches a stream specifier.
 *
 * @param s  Corresponding format context.
 * @param st Stream from s to be checked.
 * @param spec A stream specifier of the [v|a|s|d]:[\<stream index\>] form.
 *
 * @return 1 if the stream matches, 0 if it doesn't, <0 on error
 */
int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec);

/*
#define WARN_MULTIPLE_OPT_USAGE(name, type, so, st)                            \
  {                                                                            \
    char namestr[128] = "";                                                    \
    const char *spec = so->specifier && so->specifier[0] ? so->specifier : ""; \
    for (i = 0; opt_name_##name[i]; i++)                                       \
      av_strlcatf(namestr, sizeof(namestr), "-%s%s", opt_name_##name[i],       \
                  opt_name_##name[i + 1]                                       \
                      ? (opt_name_##name[i + 2] ? ", " : " or ")               \
                      : "");                                                   \
    av_log(NULL, AV_LOG_WARNING,                                               \
           "Multiple %s options specified for stream %d, only the last "       \
           "option '-%s%s%s " SPECIFIER_OPT_FMT_##type "' will be used.\n",    \
           namestr, st->index, opt_name_##name[0], spec[0] ? ":" : "", spec,   \
           so->u.type);                                                        \
  }

#define MATCH_PER_STREAM_OPT(name, type, outvar, fmtctx, st)      \
  {                                                               \
    int i, ret, matches = 0;                                      \
    SpecifierOpt *so;                                             \
    for (i = 0; i < o->nb_##name; i++) {                          \
      char *spec = o->name[i].specifier;                          \
      if ((ret = check_stream_specifier(fmtctx, st, spec)) > 0) { \
        outvar = o->name[i].u.type;                               \
        so = &o->name[i];                                         \
        matches++;                                                \
      } else if (ret < 0)                                         \
        return ret;                                               \
    }                                                             \
    if (matches > 1) {                                            \
      WARN_MULTIPLE_OPT_USAGE(name, type, so, st);                \
    }                                                             \
    return 0;                                                     \
  }
  */

std::string warn_mulitiple_opt_usage(void *avcl, AVStream *st,
                                     const std::string &option_name,
                                     const std::vector<std::string> &vals);

int match_per_stream_opt(void *avcl, AVDictionary *opts, AVFormatContext *s,
                         AVStream *st, const std::string &option_name,
                         std::vector<std::string> &vals);

int match_per_stream_opt_one(void *avcl, AVDictionary *opts, AVFormatContext *s,
                             AVStream *st, const std::string &option_name,
                             std::string &val);

int find_codec(void *avcl, const std::string &name, enum AVMediaType type,
               bool encoder, const AVCodec *&codec, bool recast_media);

}  // namespace cv
}  // namespace kingfisher

#endif
