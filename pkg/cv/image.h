#ifndef KINGFISHER_PKG_CV_IMAGE_H_
#define KINGFISHER_PKG_CV_IMAGE_H_

#if (CV_MAJOR_VERSION >= 4)
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <opencv2/imgproc/types_c.h>

#include <opencv2/opencv.hpp>
#else
#include <opencv2/opencv.hpp>
#endif

#include "Magick++.h"

namespace kingfisher {
namespace cv {
class Image {
 public:
  enum ColorSpace {
    UnknownColorSpace = 0,
    BGRColorSpace = 1,
    BGRAColorSpace = 2,
    GRAYColorSpace = 3,
    GRAYAColorSpace = 4
  };

  static int GlobalInit();

  static int DecodeImage(const std::string &image, ColorSpace targetColorSpace,
                         ::cv::Mat &matOutput);

  static int ConvertImage(Magick::Image &image, ColorSpace targetColorSpace,
                          ::cv::Mat &matOutput);
};
}  // namespace cv
}  // namespace kingfisher

#endif
