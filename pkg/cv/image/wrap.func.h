#ifndef KINGFISHER_PKG_CV_WRAP_FUNC_H_
#define KINGFISHER_PKG_CV_WRAP_FUNC_H_

#include <opencv2/opencv.hpp>

#include "Magick++.h"
#include "glog/logging.h"

namespace kingfisher {
namespace kcv {

// Wrap call to f (args), try catch exception
// https://stackoverflow.com/questions/51451843/creating-a-template-to-wrap-c-member-functions-and-expose-as-c-callbacks
template <typename Func, typename... Args>
ssize_t WrapMagickFuncT(Func f, Args... args) {
  try {
    f(args...);
  } catch (Magick::Warning &w) {
    LOG(WARNING) << "magick warning: " << w.what();
  } catch (Magick::Error &e) {
    LOG(ERROR) << "a Magick++ error occurred: " << e.what();
    return -1;
  } catch (...) {
    LOG(ERROR) << "an unhandled error has occurred";
    return -1;
  }

  return 0;
}

template <typename Func, typename... Args>
ssize_t WrapMagickFuncTWithMsg(std::string &msg, Func f, Args... args) {
  try {
    f(args...);
  } catch (Magick::Warning &w) {
    LOG(WARNING) << "magick warning: " << w.what();
    msg = w.what();
  } catch (Magick::Error &e) {
    LOG(ERROR) << "a Magick++ error occurred: " << e.what();
    msg = e.what();
    return -1;
  } catch (...) {
    LOG(ERROR) << "an unhandled error has occurred";
    return -1;
  }

  return 0;
}

template <typename Func, typename... Args>
ssize_t WrapOpencvFuncT(Func f, Args... args) {
  try {
    f(args...);
  } catch (cv::Exception &e) {
    LOG(ERROR) << "a opencv error occurred: " << e.what();
    return -1;
  } catch (...) {
    LOG(ERROR) << "an unhandled error has occurred";
    return -1;
  }

  return 0;
}

}  // namespace kcv
}  // namespace kingfisher

#endif
