#ifndef KINGFISHER_PKG_CV_VIDEO_FFMPEG_ERROR_H_
#define KINGFISHER_PKG_CV_VIDEO_FFMPEG_ERROR_H_

#include <string>

extern "C" {
#include "libavutil/error.h"
#include "libavutil/timestamp.h"
}

// fix error: "av_err2str" redefined [-Werror], temporary array error in c++1x
#ifdef av_err2str
#undef av_err2str

inline std::string av_err2string(int errnum) {
  char str[AV_ERROR_MAX_STRING_SIZE] = {0};
  return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

#define av_err2str(errnum) av_err2string(errnum).c_str()
#endif

#ifdef av_ts2timestr
#undef av_ts2timestr

inline char *av_ts2timestring(int64_t ts, AVRational *tb) {
  char buf[AV_TS_MAX_STRING_SIZE]{0};
  return av_ts_make_time_string(buf, ts, tb);
}
#define av_ts2timestr(ts, tb) av_ts2timestring(ts, tb)
#endif

#ifdef av_ts2str
#undef av_ts2str

inline std::string av_ts2string(int errnum) {
  char str[AV_TS_MAX_STRING_SIZE]{0};
  return av_ts_make_string(str, errnum);
}
#define av_ts2str(err) av_ts2string(err).c_str()
#endif

#endif
