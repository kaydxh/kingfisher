
#include "image.h"

#include <algorithm>
#include <vector>

#include "cv/image/image.pb.h"
#include "cv/image/image_error.h"
#include "cv/types/types.h"
#include "log/config.h"

#include "wrap.func.h"

namespace kingfisher {
namespace kcv {

// ==================== GaussianBlur ====================

int Image::GaussianBlur(const cv::Mat &input, cv::Mat &output,
                        int kernelSize, double sigma) {
  if (input.empty()) {
    LOG(ERROR) << "GaussianBlur: input mat is empty";
    return -1;
  }
  if (kernelSize <= 0) kernelSize = 5;
  if (kernelSize % 2 == 0) kernelSize += 1;

  auto ret = WrapOpencvFuncT([&]() {
    cv::GaussianBlur(input, output, cv::Size(kernelSize, kernelSize), sigma);
  });
  if (ret != 0) {
    LOG(ERROR) << "GaussianBlur: cv::GaussianBlur failed";
    return -1;
  }

  LOG(INFO) << "GaussianBlur: kernel=" << kernelSize << ", sigma=" << sigma;
  return 0;
}

// ==================== MeanBlur ====================

int Image::MeanBlur(const cv::Mat &input, cv::Mat &output, int kernelSize) {
  if (input.empty()) {
    LOG(ERROR) << "MeanBlur: input mat is empty";
    return -1;
  }
  if (kernelSize <= 0) kernelSize = 5;
  if (kernelSize % 2 == 0) kernelSize += 1;

  auto ret = WrapOpencvFuncT([&]() {
    cv::blur(input, output, cv::Size(kernelSize, kernelSize));
  });
  if (ret != 0) {
    LOG(ERROR) << "MeanBlur: cv::blur failed";
    return -1;
  }

  LOG(INFO) << "MeanBlur: kernel=" << kernelSize;
  return 0;
}

// ==================== MedianBlur ====================

int Image::MedianBlur(const cv::Mat &input, cv::Mat &output, int kernelSize) {
  if (input.empty()) {
    LOG(ERROR) << "MedianBlur: input mat is empty";
    return -1;
  }
  if (kernelSize <= 0) kernelSize = 5;
  if (kernelSize % 2 == 0) kernelSize += 1;

  auto ret = WrapOpencvFuncT([&]() {
    cv::medianBlur(input, output, kernelSize);
  });
  if (ret != 0) {
    LOG(ERROR) << "MedianBlur: cv::medianBlur failed";
    return -1;
  }

  LOG(INFO) << "MedianBlur: kernel=" << kernelSize;
  return 0;
}

// ==================== BilateralFilter ====================

int Image::BilateralFilter(const cv::Mat &input, cv::Mat &output,
                           int d, double sigmaColor, double sigmaSpace) {
  if (input.empty()) {
    LOG(ERROR) << "BilateralFilter: input mat is empty";
    return -1;
  }

  auto ret = WrapOpencvFuncT([&]() {
    cv::bilateralFilter(input, output, d, sigmaColor, sigmaSpace);
  });
  if (ret != 0) {
    LOG(ERROR) << "BilateralFilter: cv::bilateralFilter failed";
    return -1;
  }

  LOG(INFO) << "BilateralFilter: d=" << d
            << ", sigmaColor=" << sigmaColor
            << ", sigmaSpace=" << sigmaSpace;
  return 0;
}

// ==================== BlurImage ====================

int Image::BlurImage(const cv::Mat &input, const FilterOptions &opts,
                     cv::Mat &output) {
  int kernelSize = opts.kernel_size() > 0 ? opts.kernel_size() : 5;

  switch (opts.blur_type()) {
    case BLUR_GAUSSIAN:
      return GaussianBlur(input, output, kernelSize, opts.sigma());
    case BLUR_MEAN:
      return MeanBlur(input, output, kernelSize);
    case BLUR_MEDIAN:
      return MedianBlur(input, output, kernelSize);
    case BLUR_BILATERAL:
      return BilateralFilter(input, output, kernelSize,
                             opts.sigma_color() > 0 ? opts.sigma_color() : 75,
                             opts.sigma_space() > 0 ? opts.sigma_space() : 75);
    default:
      LOG(ERROR) << "BlurImage: unsupported blur type: " << opts.blur_type();
      return -1;
  }
}

// ==================== Sharpen ====================

int Image::Sharpen(const cv::Mat &input, cv::Mat &output,
                   double amount, int kernelSize) {
  if (input.empty()) {
    LOG(ERROR) << "Sharpen: input mat is empty";
    return -1;
  }
  if (kernelSize <= 0) kernelSize = 3;
  if (kernelSize % 2 == 0) kernelSize += 1;

  cv::Mat blurred;
  auto ret = WrapOpencvFuncT([&]() {
    cv::GaussianBlur(input, blurred, cv::Size(kernelSize, kernelSize), 0);
    cv::addWeighted(input, 1.0 + amount, blurred, -amount, 0, output);
  });
  if (ret != 0) {
    LOG(ERROR) << "Sharpen: USM sharpen failed";
    return -1;
  }

  LOG(INFO) << "Sharpen: amount=" << amount << ", kernelSize=" << kernelSize;
  return 0;
}

// ==================== AdjustBrightnessContrast ====================

int Image::AdjustBrightnessContrast(const cv::Mat &input, cv::Mat &output,
                                    double brightness, double contrast) {
  if (input.empty()) {
    LOG(ERROR) << "AdjustBrightnessContrast: input mat is empty";
    return -1;
  }

  auto ret = WrapOpencvFuncT([&]() {
    input.convertTo(output, -1, contrast, brightness);
  });
  if (ret != 0) {
    LOG(ERROR) << "AdjustBrightnessContrast: convertTo failed";
    return -1;
  }

  LOG(INFO) << "AdjustBrightnessContrast: brightness=" << brightness
            << ", contrast=" << contrast;
  return 0;
}

// ==================== Grayscale ====================

int Image::Grayscale(const cv::Mat &input, cv::Mat &output) {
  if (input.empty()) {
    LOG(ERROR) << "Grayscale: input mat is empty";
    return -1;
  }

  if (input.channels() == 1) {
    output = input.clone();
    return 0;
  }

  auto ret = WrapOpencvFuncT([&]() {
    if (input.channels() == 4) {
      cv::cvtColor(input, output, cv::COLOR_BGRA2GRAY);
    } else {
      cv::cvtColor(input, output, cv::COLOR_BGR2GRAY);
    }
  });
  if (ret != 0) {
    LOG(ERROR) << "Grayscale: cv::cvtColor failed";
    return -1;
  }

  LOG(INFO) << "Grayscale: converted " << input.channels()
            << "-channel image to grayscale";
  return 0;
}

}  // namespace kcv
}  // namespace kingfisher
