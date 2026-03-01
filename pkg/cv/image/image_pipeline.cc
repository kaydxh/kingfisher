
#include "image.h"
#include "image_pipeline.h"

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

// ======================================================================
// ImagePipeline 实现 — 面向对象链式调用 API
// ======================================================================

ImagePipeline::ImagePipeline() : error_code_(0) {}

ImagePipeline::ImagePipeline(const cv::Mat &mat)
    : mat_(mat.clone()), error_code_(0) {}

ImagePipeline::ImagePipeline(const std::string &imageData,
                             const DecodeOptions &opts)
    : error_code_(0) {
  decode(imageData, opts);
}

ImagePipeline::ImagePipeline(const ImagePipeline &other)
    : mat_(other.mat_.clone()),
      error_code_(other.error_code_),
      error_message_(other.error_message_) {}

ImagePipeline::ImagePipeline(ImagePipeline &&other) noexcept
    : mat_(std::move(other.mat_)),
      error_code_(other.error_code_),
      error_message_(std::move(other.error_message_)) {
  other.error_code_ = 0;
}

ImagePipeline& ImagePipeline::operator=(const ImagePipeline &other) {
  if (this != &other) {
    mat_ = other.mat_.clone();
    error_code_ = other.error_code_;
    error_message_ = other.error_message_;
  }
  return *this;
}

ImagePipeline& ImagePipeline::operator=(ImagePipeline &&other) noexcept {
  if (this != &other) {
    mat_ = std::move(other.mat_);
    error_code_ = other.error_code_;
    error_message_ = std::move(other.error_message_);
    other.error_code_ = 0;
  }
  return *this;
}

// ==================== 状态查询 ====================

bool ImagePipeline::ok() const { return error_code_ == 0; }

bool ImagePipeline::empty() const { return mat_.empty(); }

int ImagePipeline::error_code() const { return error_code_; }

const std::string& ImagePipeline::error_message() const {
  return error_message_;
}

const cv::Mat& ImagePipeline::mat() const { return mat_; }

cv::Mat& ImagePipeline::mat() { return mat_; }

int ImagePipeline::width() const { return mat_.cols; }

int ImagePipeline::height() const { return mat_.rows; }

int ImagePipeline::channels() const { return mat_.channels(); }

// ==================== 重置/加载 ====================

ImagePipeline& ImagePipeline::reset() {
  mat_ = cv::Mat();
  error_code_ = 0;
  error_message_.clear();
  return *this;
}

ImagePipeline& ImagePipeline::decode(const std::string &imageData,
                                     const DecodeOptions &opts) {
  if (!ok()) return *this;

  DecodeOptions effectiveOpts = opts;
  if (effectiveOpts.targetcolorspace() == UnknownColorSpace) {
    effectiveOpts.set_targetcolorspace(BGRColorSpace);
  }

  int ret = Image::DecodeImage(imageData, effectiveOpts, mat_);
  if (ret != 0) {
    setError(ret, "ImagePipeline::decode failed");
  }
  return *this;
}

ImagePipeline& ImagePipeline::decodeFile(const std::string &imageFile,
                                         const DecodeOptions &opts) {
  if (!ok()) return *this;

  int ret = Image::DecodeImageFile(imageFile, opts, mat_);
  if (ret != 0) {
    setError(ret, "ImagePipeline::decodeFile failed: " + imageFile);
  }
  return *this;
}

ImagePipeline& ImagePipeline::load(const cv::Mat &mat) {
  if (!ok()) return *this;

  if (mat.empty()) {
    setError(-1, "ImagePipeline::load: input mat is empty");
    return *this;
  }
  mat_ = mat.clone();
  return *this;
}

// ==================== 几何变换 ====================

ImagePipeline& ImagePipeline::rotate(double angle) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::RotateImage(mat_, angle, output);
  if (ret != 0) {
    setError(ret, "ImagePipeline::rotate failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::resize(int width, int height, bool keepRatio) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::ResizeImage(mat_, width, height, keepRatio, output);
  if (ret != 0) {
    setError(ret, "ImagePipeline::resize failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::crop(const Rect &rect) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::CropImage(mat_, rect, output);
  if (ret != 0) {
    setError(ret, "ImagePipeline::crop failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::centerCrop(int width, int height) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::CenterCropImage(mat_, width, height, output);
  if (ret != 0) {
    setError(ret, "ImagePipeline::centerCrop failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::flip(FlipMode mode) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::FlipImage(mat_, mode, output);
  if (ret != 0) {
    setError(ret, "ImagePipeline::flip failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::thumbnail(int maxDimension) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::Thumbnail(mat_, maxDimension, output);
  if (ret != 0) {
    setError(ret, "ImagePipeline::thumbnail failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

// ==================== 颜色/滤镜 ====================

ImagePipeline& ImagePipeline::convertColorSpace(ColorSpace from,
                                                 ColorSpace to) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::ConvertColorSpace(mat_, from, to, output);
  if (ret != 0) {
    setError(ret, "ImagePipeline::convertColorSpace failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::grayscale() {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::Grayscale(mat_, output);
  if (ret != 0) {
    setError(ret, "ImagePipeline::grayscale failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::gaussianBlur(int kernelSize, double sigma) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::GaussianBlur(mat_, output, kernelSize, sigma);
  if (ret != 0) {
    setError(ret, "ImagePipeline::gaussianBlur failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::meanBlur(int kernelSize) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::MeanBlur(mat_, output, kernelSize);
  if (ret != 0) {
    setError(ret, "ImagePipeline::meanBlur failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::medianBlur(int kernelSize) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::MedianBlur(mat_, output, kernelSize);
  if (ret != 0) {
    setError(ret, "ImagePipeline::medianBlur failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::bilateralFilter(int d, double sigmaColor,
                                               double sigmaSpace) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::BilateralFilter(mat_, output, d, sigmaColor, sigmaSpace);
  if (ret != 0) {
    setError(ret, "ImagePipeline::bilateralFilter failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::sharpen(double amount, int kernelSize) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::Sharpen(mat_, output, amount, kernelSize);
  if (ret != 0) {
    setError(ret, "ImagePipeline::sharpen failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

ImagePipeline& ImagePipeline::adjustBrightnessContrast(double brightness,
                                                        double contrast) {
  if (!ok()) return *this;

  cv::Mat output;
  int ret = Image::AdjustBrightnessContrast(mat_, output, brightness, contrast);
  if (ret != 0) {
    setError(ret, "ImagePipeline::adjustBrightnessContrast failed");
    return *this;
  }
  mat_ = output;
  return *this;
}

// ==================== 合成/标注 ====================

ImagePipeline& ImagePipeline::overlay(const cv::Mat &src, int x, int y,
                                       double alpha) {
  if (!ok()) return *this;

  int ret = Image::Overlay(mat_, src, x, y, alpha);
  if (ret != 0) {
    setError(ret, "ImagePipeline::overlay failed");
  }
  return *this;
}

ImagePipeline& ImagePipeline::watermark(const cv::Mat &logo, cv::Rect region,
                                         int interpolation, double alpha) {
  if (!ok()) return *this;

  int ret = Image::AdaptiveWatermarkFill(mat_, logo, region,
                                          interpolation, alpha);
  if (ret != 0) {
    setError(ret, "ImagePipeline::watermark failed");
  }
  return *this;
}

ImagePipeline& ImagePipeline::annotate(const std::string &text,
                                        const cv::Point &position,
                                        const AnnotateOptions &opts) {
  if (!ok()) return *this;

  int ret = Image::AnnotateImage(mat_, text, position, opts);
  if (ret != 0) {
    setError(ret, "ImagePipeline::annotate failed");
  }
  return *this;
}

// ==================== 输出 ====================

std::string ImagePipeline::encode(const std::string &format, int quality) {
  if (!ok()) return "";

  std::string output;
  int ret = Image::EncodeImage(mat_, format, quality, output);
  if (ret != 0) {
    setError(ret, "ImagePipeline::encode failed");
    return "";
  }
  return output;
}

int ImagePipeline::writeTo(const std::string &path) {
  if (!ok()) return error_code_;
  return Image::WriteImage(mat_, path);
}

int ImagePipeline::writeTo(const std::string &path,
                            const std::vector<int> &params) {
  if (!ok()) return error_code_;
  return Image::WriteImage(mat_, path, params);
}

ImagePipeline ImagePipeline::clone() const {
  ImagePipeline result;
  result.mat_ = mat_.clone();
  result.error_code_ = error_code_;
  result.error_message_ = error_message_;
  return result;
}

void ImagePipeline::setError(int code, const std::string &msg) {
  error_code_ = code;
  error_message_ = msg;
  LOG(ERROR) << msg << " (error_code=" << code << ")";
}

}  // namespace kcv
}  // namespace kingfisher
