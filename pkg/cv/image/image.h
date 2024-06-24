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
#include "image.pb.h"

namespace kingfisher {
namespace kcv {

class Image {
 public:
  static int GlobalInit();
  static int GlobalRelease();

  static int PingImage(ImageInfo &result, const std::string &imageData);

  static int DecodeImageFile(const std::string &imageFile,
                             ::cv::Mat &matOutput);

  static int DecodeImageFile(const std::string &imageFile,
                             const DecodeOptions &opts, ::cv::Mat &matOutput);

  static int DecodeImage(const std::string &imageData,
                         const DecodeOptions &opts, ::cv::Mat &matOutput);

  static int RotateImage(const std::string &imageData, double degree,
                         ::cv::Mat &matOutput);

  static int ResizeImage(const std::string &imageData, int width, int height,
                         bool keepRatio, ::cv::Mat &matOutput);

  static int CropImage(const std::string &imageData, const Rect &rect,
                       ::cv::Mat &matOutput);

  static int WriteImage(const cv::Mat &mat, const std::string &path);

  static int DumpImageFileToBytes(const std::string &imageFile,
                                  const std::string &path);

  static int DumpImageToBytes(const cv::Mat &mat, const std::string &path);
  // static int AnnotateImage(const std::string &imageData,
  //                        const std::string &text, const Rect &rect,
  //                       ::cv::Mat &matOutput);
  // static int ZoomImage(const std::string &imageData, double ratio,
  //                     ::cv::Mat &matOutput);
};
}  // namespace kcv
}  // namespace kingfisher

#endif
