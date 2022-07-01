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
#include "types.h"

namespace kingfisher {
namespace cv {

class Image {
 public:
  static int GlobalInit();

  static int DecodeImage(const std::string &image, ColorSpace targetColorSpace,
                         bool autoOrient, ::cv::Mat &matOutput);
  static int RotateImage(const std::string &imageData, double degree,
                         ::cv::Mat &matOutput);

  static int ResizeImage(const std::string &imageData, int width, int height,
                         bool keepRatio, ::cv::Mat &matOutput);
  // static int ConvertImage(Magick::Image &image, ColorSpace targetColorSpace,
  //                        ::cv::Mat &matOutput);
};
}  // namespace cv
}  // namespace kingfisher

#endif
