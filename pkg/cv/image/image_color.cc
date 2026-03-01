
#include "image.h"

#include <algorithm>
#include <vector>

#include "cv/image/image.pb.h"
#include "cv/image/image_error.h"
#include "cv/types/types.h"
#include "log/config.h"

#ifdef ENABLE_OPENCV
#include "opencv2/highgui/highgui.hpp"
#endif

#include "wrap.func.h"

namespace kingfisher {
namespace kcv {

// 统一 OpenCV 2.x/3.x/4.x 的转换标识
#if CV_VERSION_MAJOR < 4
#define COLOR_RGBA2BGR CV_RGBA2BGR
#define COLOR_BGR2BGRA CV_BGR2BGRA
#define COLOR_GRAY2BGRA CV_GRAY2BGRA
#define COLOR_GRAY2BGR CV_GRAY2BGR
#define COLOR_BGR2GRAY CV_BGR2GRAY
#define COLOR_RGB2BGR CV_RGB2BGR
#define COLOR_BGR2RGB CV_BGR2RGB
#define COLOR_RGBA2BGRA CV_RGBA2BGRA
#define COLOR_BGRA2RGBA CV_BGRA2RGBA
#define COLOR_GRAY2RGB CV_GRAY2RGB
#define COLOR_RGB2GRAY CV_RGB2GRAY
#define COLOR_RGBA2BGR CV_RGBA2BGR
#define COLOR_BGR2RGBA CV_BGR2RGBA
#define COLOR_RGB2BGRA CV_RGB2BGRA
#define COLOR_BGRA2RGB CV_BGRA2RGB

#ifndef COLOR_BGRA2GRAY
#define COLOR_BGRA2GRAY CV_BGRA2GRAY
#endif

#ifndef COLOR_BGR2HSV
#define COLOR_BGR2HSV CV_BGR2HSV
#endif
#endif

// ==================== 颜色空间转换辅助函数 ====================

static int GetCvtColorCode(ColorSpace from, ColorSpace to) {
  if (from == BGRColorSpace && to == GRAYColorSpace) return cv::COLOR_BGR2GRAY;
  if (from == BGRColorSpace && to == BGRAColorSpace) return cv::COLOR_BGR2BGRA;
  if (from == BGRColorSpace && to == RGBColorSpace) return cv::COLOR_BGR2RGB;
  if (from == BGRColorSpace && to == RGBAColorSpace) return cv::COLOR_BGR2RGBA;

  if (from == BGRAColorSpace && to == BGRColorSpace) return cv::COLOR_BGRA2BGR;
  if (from == BGRAColorSpace && to == GRAYColorSpace) return cv::COLOR_BGRA2GRAY;
  if (from == BGRAColorSpace && to == RGBColorSpace) return cv::COLOR_BGRA2RGB;
  if (from == BGRAColorSpace && to == RGBAColorSpace) return cv::COLOR_BGRA2RGBA;

  if (from == GRAYColorSpace && to == BGRColorSpace) return cv::COLOR_GRAY2BGR;
  if (from == GRAYColorSpace && to == BGRAColorSpace) return cv::COLOR_GRAY2BGRA;
  if (from == GRAYColorSpace && to == RGBColorSpace) return cv::COLOR_GRAY2RGB;

  if (from == RGBColorSpace && to == BGRColorSpace) return cv::COLOR_RGB2BGR;
  if (from == RGBColorSpace && to == GRAYColorSpace) return cv::COLOR_RGB2GRAY;
  if (from == RGBColorSpace && to == BGRAColorSpace) return cv::COLOR_RGB2BGRA;

  if (from == RGBAColorSpace && to == BGRColorSpace) return cv::COLOR_RGBA2BGR;
  if (from == RGBAColorSpace && to == BGRAColorSpace) return cv::COLOR_RGBA2BGRA;

  return kImageUnsupportedColorSpace;
}

// ==================== ConvertColorSpace ====================

int Image::ConvertColorSpace(const cv::Mat &input, ColorSpace from,
                             ColorSpace to, cv::Mat &output) {
  if (input.empty()) {
    LOG(ERROR) << "ConvertColorSpace: input mat is empty";
    return kImageEmptyInput;
  }

  if (from == to) {
    output = input.clone();
    return 0;
  }

  int code = GetCvtColorCode(from, to);
  if (code < 0) {
    LOG(ERROR) << "ConvertColorSpace: unsupported conversion from "
               << from << " to " << to;
    return kImageUnsupportedColorSpace;
  }

  auto ret = WrapOpencvFuncT([&]() {
    cv::cvtColor(input, output, code);
  });
  if (ret != 0) {
    LOG(ERROR) << "ConvertColorSpace: cv::cvtColor failed";
    return kImageUnsupportedOperation;
  }

  LOG(INFO) << "ConvertColorSpace: converted from " << from << " to " << to;
  return 0;
}

// ==================== HConcat / VConcat ====================

int Image::HConcat(const std::vector<cv::Mat> &images, cv::Mat &output) {
  if (images.empty()) {
    LOG(ERROR) << "HConcat: input images list is empty";
    return kImageEmptyInput;
  }
  if (images.size() == 1) {
    output = images[0].clone();
    return 0;
  }

  int type = images[0].type();
  for (size_t i = 1; i < images.size(); ++i) {
    if (images[i].empty()) {
      LOG(ERROR) << "HConcat: image at index " << i << " is empty";
      return kImageEmptyInput;
    }
    if (images[i].type() != type) {
      LOG(ERROR) << "HConcat: image type mismatch at index " << i;
      return kImageTypeMismatch;
    }
  }

  int maxHeight = 0;
  for (const auto &img : images) {
    maxHeight = std::max(maxHeight, img.rows);
  }

  std::vector<cv::Mat> resized;
  for (const auto &img : images) {
    if (img.rows == maxHeight) {
      resized.push_back(img);
    } else {
      cv::Mat tmp;
      double ratio = static_cast<double>(maxHeight) / img.rows;
      cv::resize(img, tmp, cv::Size(static_cast<int>(img.cols * ratio), maxHeight));
      resized.push_back(tmp);
    }
  }

  auto ret = WrapOpencvFuncT([&]() {
    cv::hconcat(resized, output);
  });
  if (ret != 0) {
    LOG(ERROR) << "HConcat: cv::hconcat failed";
    return kImageUnsupportedOperation;
  }

  LOG(INFO) << "HConcat: concatenated " << images.size()
            << " images horizontally, output size=" << output.cols
            << "x" << output.rows;
  return 0;
}

int Image::VConcat(const std::vector<cv::Mat> &images, cv::Mat &output) {
  if (images.empty()) {
    LOG(ERROR) << "VConcat: input images list is empty";
    return kImageEmptyInput;
  }
  if (images.size() == 1) {
    output = images[0].clone();
    return 0;
  }

  int type = images[0].type();
  for (size_t i = 1; i < images.size(); ++i) {
    if (images[i].empty()) {
      LOG(ERROR) << "VConcat: image at index " << i << " is empty";
      return kImageEmptyInput;
    }
    if (images[i].type() != type) {
      LOG(ERROR) << "VConcat: image type mismatch at index " << i;
      return kImageTypeMismatch;
    }
  }

  int maxWidth = 0;
  for (const auto &img : images) {
    maxWidth = std::max(maxWidth, img.cols);
  }

  std::vector<cv::Mat> resized;
  for (const auto &img : images) {
    if (img.cols == maxWidth) {
      resized.push_back(img);
    } else {
      cv::Mat tmp;
      double ratio = static_cast<double>(maxWidth) / img.cols;
      cv::resize(img, tmp, cv::Size(maxWidth, static_cast<int>(img.rows * ratio)));
      resized.push_back(tmp);
    }
  }

  auto ret = WrapOpencvFuncT([&]() {
    cv::vconcat(resized, output);
  });
  if (ret != 0) {
    LOG(ERROR) << "VConcat: cv::vconcat failed";
    return kImageUnsupportedOperation;
  }

  LOG(INFO) << "VConcat: concatenated " << images.size()
            << " images vertically, output size=" << output.cols
            << "x" << output.rows;
  return 0;
}

// ==================== Overlay ====================

int Image::Overlay(cv::Mat &dest, const cv::Mat &src, int x, int y,
                   double alpha) {
  if (dest.empty() || src.empty()) {
    LOG(ERROR) << "Overlay: dest or src mat is empty";
    return kImageEmptyInput;
  }
  if (alpha < 0.0 || alpha > 1.0) {
    LOG(ERROR) << "Overlay: alpha value out of range [0, 1]: " << alpha;
    return kImageInvalidParam;
  }

  int srcX = 0, srcY = 0;
  int dstX = x, dstY = y;
  int overlapW = src.cols, overlapH = src.rows;

  if (dstX < 0) {
    srcX = -dstX;
    overlapW += dstX;
    dstX = 0;
  }
  if (dstY < 0) {
    srcY = -dstY;
    overlapH += dstY;
    dstY = 0;
  }

  if (dstX + overlapW > dest.cols) {
    overlapW = dest.cols - dstX;
  }
  if (dstY + overlapH > dest.rows) {
    overlapH = dest.rows - dstY;
  }

  if (overlapW <= 0 || overlapH <= 0) {
    LOG(WARNING) << "Overlay: no overlap region, skipping";
    return 0;
  }

  cv::Rect srcROI(srcX, srcY, overlapW, overlapH);
  cv::Rect dstROI(dstX, dstY, overlapW, overlapH);

  cv::Mat srcRegion = src(srcROI);
  cv::Mat destRegion = dest(dstROI);

  cv::Mat srcAligned;
  if (srcRegion.channels() != destRegion.channels()) {
    if (destRegion.channels() == 4 && srcRegion.channels() == 3) {
      cv::cvtColor(srcRegion, srcAligned, cv::COLOR_BGR2BGRA);
    } else if (destRegion.channels() == 3 && srcRegion.channels() == 4) {
      cv::cvtColor(srcRegion, srcAligned, cv::COLOR_BGRA2BGR);
    } else if (destRegion.channels() == 3 && srcRegion.channels() == 1) {
      cv::cvtColor(srcRegion, srcAligned, cv::COLOR_GRAY2BGR);
    } else {
      srcAligned = srcRegion;
    }
  } else {
    srcAligned = srcRegion;
  }

  cv::Mat blended;
  cv::addWeighted(destRegion, 1.0 - alpha, srcAligned, alpha, 0.0, blended);
  blended.copyTo(destRegion);

  LOG(INFO) << "Overlay: overlaid at (" << x << ", " << y
            << "), overlap=" << overlapW << "x" << overlapH
            << ", alpha=" << alpha;
  return 0;
}

// ==================== AnnotateImage ====================

int Image::AnnotateImage(cv::Mat &image, const std::string &text,
                         const cv::Point &position,
                         const AnnotateOptions &opts) {
  if (image.empty()) {
    LOG(ERROR) << "AnnotateImage: input image is empty";
    return kImageEmptyInput;
  }
  if (text.empty()) {
    LOG(WARNING) << "AnnotateImage: text is empty, nothing to draw";
    return 0;
  }

  cv::Scalar color(opts.color_b(), opts.color_g(), opts.color_r());

  double fontSize = opts.font_size() > 0 ? opts.font_size() : 1.0;
  int thickness = opts.thickness() > 0 ? opts.thickness() : 1;
  int fontFace = cv::FONT_HERSHEY_SIMPLEX;
  if (opts.anti_alias()) {
    fontFace |= cv::FONT_ITALIC;
  }
#if CV_MAJOR_VERSION >= 4
  int lineType = opts.anti_alias() ? cv::LINE_AA : cv::LINE_8;
#else
  int lineType = opts.anti_alias() ? CV_AA : 8;
#endif

  auto ret = WrapOpencvFuncT([&]() {
    cv::putText(image, text, position, fontFace, fontSize, color,
                thickness, lineType);
  });
  if (ret != 0) {
    LOG(ERROR) << "AnnotateImage: cv::putText failed";
    return kImageUnsupportedOperation;
  }

  LOG(INFO) << "AnnotateImage: drew text \"" << text << "\" at ("
            << position.x << ", " << position.y << ")";
  return 0;
}

int Image::AnnotateImage(const std::string &imageData,
                         const std::string &text, const cv::Point &position,
                         const AnnotateOptions &opts, cv::Mat &matOutput) {
  DecodeOptions decOpts;
  decOpts.set_targetcolorspace(BGRColorSpace);
  decOpts.set_auto_orient(true);
  int ret = DecodeImage(imageData, decOpts, matOutput);
  if (ret != 0) {
    LOG(ERROR) << "AnnotateImage(string): failed to decode image";
    return ret;
  }
  return AnnotateImage(matOutput, text, position, opts);
}

}  // namespace kcv
}  // namespace kingfisher
