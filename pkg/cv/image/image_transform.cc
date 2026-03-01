
#include "image.h"

#ifdef ENABLE_GRAPHICS_MAGICK
#include <Magick++.h>
#include <magick/api.h>
#endif

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>

#include "cv/image/image.pb.h"
#include "cv/image/image_error.h"
#include "cv/types/types.h"
#include "log/config.h"

#ifdef ENABLE_OPENCV
#include "opencv2/highgui/highgui.hpp"
#endif

#include "wrap.func.h"

#ifdef ENABLE_GRAPHICS_MAGICK
using namespace Magick;
using namespace MagickLib;
#endif

namespace kingfisher {
namespace kcv {

// 内部辅助函数（与 image_codec.cc 中的定义一致，此处重复定义供本编译单元使用）
static int ImageToMat(Magick::Image &image, ColorSpace targetColorSpace,
                      ::cv::Mat &matOutput) {
  if (!image.isValid()) {
    return -1;
  }
  int w = image.columns();
  int h = image.rows();

  image.colorSpace(image.matte() ? Magick::TransparentColorspace
                                 : Magick::RGBColorspace);
  if (image.matte()) {
    matOutput = cv::Mat(h, w, CV_8UC4);
  } else {
    matOutput = cv::Mat(h, w, CV_8UC3);
  }

  std::string map = "BGRA";
  switch (targetColorSpace) {
    case BGRColorSpace:
    case BGRAColorSpace:
      if (image.matte()) {
        map = "BGRA";
      } else {
        map = "BGR";
      }
      break;
    case GRAYColorSpace:
    case GRAYAColorSpace:
      if (image.matte()) {
        image.type(Magick::GrayscaleMatteType);
        map = "BGRA";
      } else {
        image.type(Magick::GrayscaleType);
        map = "BGR";
      }
      break;
    default:
      break;
  }

  image.write(0, 0, w, h, map, Magick::CharPixel, matOutput.data);
  return 0;
}

static int ImageToMat(Magick::Image &image, ::cv::Mat &matOutput) {
  return ImageToMat(image, BGRAColorSpace, matOutput);
}

static int imageRead(const std::string &imageData, Magick::Image &imageOutput) {
  auto ret = WrapMagickFuncT([&]() {
    Magick::Blob blob((void *)imageData.data(), imageData.length());
    imageOutput.read(blob);
  });
  if (ret != 0) {
    return ret;
  }

  if (!imageOutput.isValid()) {
    return -1;
  }

  return 0;
}

// ==================== RotateImage ====================

int Image::RotateImage(const std::string &imageData, double angle,
                       ::cv::Mat &matOutput) {
  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    return ret;
  }

  if (angle) {
    ret = WrapMagickFuncT([&]() { image.rotate(angle); });
    if (ret != 0) {
      return ret;
    }
  }

  return ImageToMat(image, matOutput);
}

// 统一 OpenCV 2.x/3.x/4.x 的转换标识（本编译单元需要）
#if CV_VERSION_MAJOR < 4
#define COLOR_BGR2GRAY CV_BGR2GRAY
#endif

int Image::RotateImage(const ::cv::Mat &matInput, double angle,
                       ::cv::Mat &matOutput) {
#if 0
  Magick::Image image(matInput.cols, matInput.rows, "BGR", CharPixel,
                      matInput.data);

  if (degree) {
    int ret = WrapMagickFuncT([&]() { image.rotate(degree); });
    if (ret != 0) {
      return ret;
    }
  }

  return ImageToMat(image, matOutput);
#endif
  if (angle == ROTATE_CLOCKWISE_0 || angle == ROTATE_CLOCKWISE_360) {
    matOutput = matInput.clone();
    LOG(INFO) << "no need to rotate, rotate angle=" << angle;
    return 0;
  }
  if (angle == ROTATE_CLOCKWISE_90) {
    cv::transpose(matInput, matOutput);
    cv::flip(matOutput, matOutput, 1);
    LOG(INFO) << "rotate origin image by angle=" << angle;
    return 0;
  }
  if (angle == ROTATE_CLOCKWISE_180) {
    cv::flip(matInput, matOutput, -1);
    LOG(INFO) << "rotate origin image by angle=" << angle;
    return 0;
  }
  if (angle == ROTATE_CLOCKWISE_270) {
    cv::transpose(matInput, matOutput);
    cv::flip(matOutput, matOutput, 0);
    LOG(INFO) << "rotate origin image by angle=" << angle;
    return 0;
  }

  // 任意角度旋转：使用 getRotationMatrix2D + warpAffine
  cv::Point2f center(matInput.cols / 2.0f, matInput.rows / 2.0f);
  cv::Mat rot = cv::getRotationMatrix2D(center, -angle, 1.0);

  // 计算旋转后的包围框大小
  cv::Rect bbox = cv::RotatedRect(cv::Point2f(), matInput.size(), static_cast<float>(angle)).boundingRect();

  rot.at<double>(0, 2) += bbox.width / 2.0 - matInput.cols / 2.0;
  rot.at<double>(1, 2) += bbox.height / 2.0 - matInput.rows / 2.0;

  cv::warpAffine(matInput, matOutput, rot, bbox.size());
  LOG(INFO) << "rotate origin image by arbitrary angle=" << angle
            << ", output size=" << matOutput.cols << "x" << matOutput.rows;
  return 0;
}

// ==================== ResizeImage ====================

int Image::ResizeImage(const std::string &imageData, int width, int height,
                       bool keepRatio, ::cv::Mat &matOutput) {
  if (width <= 0 || height <= 0) {
    return -1;
  }

  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    return ret;
  }

  int w0 = image.columns();
  int h0 = image.rows();
  if (keepRatio) {
    if (width > height) {
      height = static_cast<double>(h0 * width) / w0;
    } else {
      width = static_cast<double>(w0 * height) / h0;
    }
  }

  char buf[64] = {0};
  snprintf(buf, sizeof(buf), "%dx%d!", width, height);
  image.zoom(buf);

  return ImageToMat(image, matOutput);
}

int Image::ResizeImage(const cv::Mat &matInput, int width, int height,
                       bool keepRatio, cv::Mat &matOutput) {
  if (matInput.empty()) {
    LOG(ERROR) << "ResizeImage(Mat): input mat is empty";
    return -1;
  }
  if (width <= 0 || height <= 0) {
    LOG(ERROR) << "ResizeImage(Mat): invalid target size: "
               << width << "x" << height;
    return -1;
  }

  int w0 = matInput.cols;
  int h0 = matInput.rows;

  if (keepRatio) {
    if (width > height) {
      height = static_cast<int>(static_cast<double>(h0 * width) / w0);
    } else {
      width = static_cast<int>(static_cast<double>(w0 * height) / h0);
    }
  }

  auto ret = WrapOpencvFuncT([&]() {
    cv::resize(matInput, matOutput, cv::Size(width, height));
  });
  if (ret != 0) {
    LOG(ERROR) << "ResizeImage(Mat): cv::resize failed";
    return -1;
  }

  LOG(INFO) << "ResizeImage(Mat): resized from " << w0 << "x" << h0
            << " to " << width << "x" << height;
  return 0;
}

// ==================== CropImage ====================

int Image::CropImage(const std::string &imageData, const Rect &rect,
                     ::cv::Mat &matOutput) {
  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    return ret;
  }
  int w0 = image.columns();
  int h0 = image.rows();

  auto rect0 = ::cv::Rect(0, 0, w0, h0);
  auto intesectRect =
      rect0 & cv::Rect(rect.x(), rect.y(), rect.width(), rect.height());
  image.crop(Magick::Geometry(intesectRect.width, intesectRect.height,
                              intesectRect.x, intesectRect.y));
  return ImageToMat(image, matOutput);
}

int Image::CropImage(const cv::Mat &matInput, const Rect &rect,
                     cv::Mat &matOutput) {
  if (matInput.empty()) {
    LOG(ERROR) << "CropImage(Mat): input mat is empty";
    return -1;
  }

  auto rect0 = cv::Rect(0, 0, matInput.cols, matInput.rows);
  auto cropRect =
      rect0 & cv::Rect(rect.x(), rect.y(), rect.width(), rect.height());

  if (cropRect.width <= 0 || cropRect.height <= 0) {
    LOG(ERROR) << "CropImage(Mat): invalid crop region, intersection is empty";
    return -1;
  }

  matOutput = matInput(cropRect).clone();
  LOG(INFO) << "CropImage(Mat): cropped region (" << cropRect.x << ", "
            << cropRect.y << ", " << cropRect.width << ", " << cropRect.height
            << ")";
  return 0;
}

// ==================== CenterCropImage ====================

int Image::CenterCropImage(const cv::Mat &matInput, int width, int height,
                            cv::Mat &matOutput) {
  if (matInput.empty()) {
    LOG(ERROR) << "CenterCropImage: input mat is empty";
    return -1;
  }
  if (width <= 0 || height <= 0) {
    LOG(ERROR) << "CenterCropImage: invalid crop size: "
               << width << "x" << height;
    return -1;
  }

  int cropW = std::min(width, matInput.cols);
  int cropH = std::min(height, matInput.rows);

  int x = (matInput.cols - cropW) / 2;
  int y = (matInput.rows - cropH) / 2;

  cv::Rect cropRect(x, y, cropW, cropH);
  matOutput = matInput(cropRect).clone();

  LOG(INFO) << "CenterCropImage: center cropped to " << cropW << "x" << cropH;
  return 0;
}

// ==================== FlipImage ====================

int Image::FlipImage(const cv::Mat &input, FlipMode mode, cv::Mat &output) {
  if (input.empty()) {
    LOG(ERROR) << "FlipImage: input mat is empty";
    return -1;
  }

  int flipCode;
  switch (mode) {
    case FLIP_HORIZONTAL:
      flipCode = 1;
      break;
    case FLIP_VERTICAL:
      flipCode = 0;
      break;
    case FLIP_BOTH:
      flipCode = -1;
      break;
    default:
      LOG(ERROR) << "FlipImage: unsupported flip mode: " << mode;
      return -1;
  }

  auto ret = WrapOpencvFuncT([&]() {
    cv::flip(input, output, flipCode);
  });
  if (ret != 0) {
    LOG(ERROR) << "FlipImage: cv::flip failed";
    return -1;
  }

  LOG(INFO) << "FlipImage: flipped with mode=" << mode;
  return 0;
}

int Image::FlipImage(const std::string &imageData, FlipMode mode,
                     cv::Mat &output) {
  cv::Mat mat;
  DecodeOptions opts;
  opts.set_targetcolorspace(BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = DecodeImage(imageData, opts, mat);
  if (ret != 0) {
    LOG(ERROR) << "FlipImage(string): failed to decode image";
    return ret;
  }
  return FlipImage(mat, mode, output);
}

// ==================== Thumbnail ====================

int Image::Thumbnail(const std::string &imageData, int maxDimension,
                     cv::Mat &matOutput) {
  if (maxDimension <= 0) {
    LOG(ERROR) << "Thumbnail: invalid maxDimension: " << maxDimension;
    return -1;
  }

  cv::Mat mat;
  DecodeOptions opts;
  opts.set_targetcolorspace(BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = DecodeImage(imageData, opts, mat);
  if (ret != 0) {
    LOG(ERROR) << "Thumbnail(string): failed to decode image";
    return ret;
  }
  return Thumbnail(mat, maxDimension, matOutput);
}

int Image::Thumbnail(const cv::Mat &matInput, int maxDimension,
                     cv::Mat &matOutput) {
  if (matInput.empty()) {
    LOG(ERROR) << "Thumbnail: input mat is empty";
    return -1;
  }
  if (maxDimension <= 0) {
    LOG(ERROR) << "Thumbnail: invalid maxDimension: " << maxDimension;
    return -1;
  }

  int w = matInput.cols;
  int h = matInput.rows;

  if (w <= maxDimension && h <= maxDimension) {
    matOutput = matInput.clone();
    LOG(INFO) << "Thumbnail: image already within max dimension, no resize needed";
    return 0;
  }

  double ratio;
  if (w > h) {
    ratio = static_cast<double>(maxDimension) / w;
  } else {
    ratio = static_cast<double>(maxDimension) / h;
  }

  int newW = static_cast<int>(w * ratio);
  int newH = static_cast<int>(h * ratio);

  auto ret = WrapOpencvFuncT([&]() {
    cv::resize(matInput, matOutput, cv::Size(newW, newH), 0, 0, cv::INTER_AREA);
  });
  if (ret != 0) {
    LOG(ERROR) << "Thumbnail: cv::resize failed";
    return -1;
  }

  LOG(INFO) << "Thumbnail: resized from " << w << "x" << h
            << " to " << newW << "x" << newH
            << " (maxDimension=" << maxDimension << ")";
  return 0;
}

}  // namespace kcv
}  // namespace kingfisher
