
#ifndef KINGFISHER_PKG_CV_WRAP_FUNC_H_
#define KINGFISHER_PKG_CV_WRAP_FUNC_H_

/**
 * @file wrap.func.h
 * @brief 通用异常包装模板函数
 *
 * 提供 GraphicsMagick 和 OpenCV 操作的异常安全包装。
 * 位于 pkg/cv/ 公共目录，供所有 cv 子模块（image/geometry/video 等）复用。
 */

#include <string>

#ifdef ENABLE_OPENCV
#include <opencv2/opencv.hpp>
#endif

#ifdef ENABLE_GRAPHICS_MAGICK
#include "Magick++.h"
#endif

#include "glog/logging.h"

namespace kingfisher {
namespace kcv {

// ==================== GraphicsMagick 异常包装 ====================

#ifdef ENABLE_GRAPHICS_MAGICK

/**
 * @brief 包装 GraphicsMagick 操作，捕获 Magick 异常
 * @param f 要执行的函数/lambda
 * @return 0 成功，-1 失败
 */
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

/**
 * @brief 包装 GraphicsMagick 操作，同时返回警告/错误消息
 */
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

#endif  // ENABLE_GRAPHICS_MAGICK

// ==================== OpenCV 异常包装 ====================

#ifdef ENABLE_OPENCV

/**
 * @brief 包装 OpenCV 操作，捕获 cv::Exception 异常
 * @param f 要执行的函数/lambda
 * @return 0 成功，-1 失败
 */
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

#endif  // ENABLE_OPENCV

}  // namespace kcv
}  // namespace kingfisher

#endif
