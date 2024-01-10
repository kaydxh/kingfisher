#ifndef KINGFISHER_PKG_CV_VIDEO_FFMPEG_ERROR_H_
#define KINGFISHER_PKG_CV_VIDEO_FFMPEG_ERROR_H_

#include <string>

extern "C" {
#include "libavutil/error.h"
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
#endif
