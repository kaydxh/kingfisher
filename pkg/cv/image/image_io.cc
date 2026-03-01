
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

// ==================== MatToImage ====================

int Image::MatToImage(const cv::Mat &mat, Magick::Image &imageOutput) {
  if (mat.empty()) {
    LOG(ERROR) << "MatToImage: input mat is empty";
    return kImageEmptyInput;
  }

  std::string map;
  int channels = mat.channels();
  switch (channels) {
    case 1:
      map = "I";
      break;
    case 3:
      map = "BGR";
      break;
    case 4:
      map = "BGRA";
      break;
    default:
      LOG(ERROR) << "MatToImage: unsupported channel count: " << channels;
      return kImageChannelMismatch;
  }

  cv::Mat continuous;
  if (mat.isContinuous()) {
    continuous = mat;
  } else {
    continuous = mat.clone();
  }

  auto ret = WrapMagickFuncT([&]() {
    imageOutput.read(mat.cols, mat.rows, map, Magick::CharPixel,
                     continuous.data);
    if (channels == 4) {
      imageOutput.matte(true);
    }
  });
  if (ret != 0) {
    LOG(ERROR) << "MatToImage: Magick::Image::read failed";
    return kImageUnsupportedOperation;
  }

  LOG(INFO) << "MatToImage: converted cv::Mat (" << mat.cols << "x" << mat.rows
            << ", " << channels << "ch) to Magick::Image";
  return 0;
}

// ==================== WriteImage ====================

int Image::WriteImage(const cv::Mat &mat, const std::string &path) {
  auto ret = WrapOpencvFuncT([&]() { cv::imwrite(path, mat); });
  if (ret != 0) {
    return ret;
  }

  return 0;
}

int Image::WriteImage(const cv::Mat &mat, const std::string &path,
                      const std::vector<int> &params) {
  auto ret = WrapOpencvFuncT([&]() { cv::imwrite(path, mat, params); });
  if (ret != 0) {
    return ret;
  }

  return 0;
}

// ==================== DumpImageToBytes ====================

int Image::DumpImageFileToBytes(const std::string &imageFile,
                                const std::string &path) {
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  opts.set_auto_orient(true);

  ::cv::Mat mat;
  int ret = DecodeImageFile(imageFile, opts, mat);
  if (ret) {
    return ret;
  }

  return DumpImageToBytes(mat, path);
}

int Image::DumpImageToBytes(const cv::Mat &mat, const std::string &path) {
  std::ofstream ofs(path);
  ofs.precision(6);
  ofs.setf(std::ios::fixed, std::ios::floatfield);
  int sz = mat.total() * mat.channels();
  for (int i = 0; i < sz; ++i) {
    ofs << static_cast<int>(static_cast<uint8_t *>(mat.data)[i]) << std::endl;
  }
  ofs.close();
  return 0;
}

// ==================== AdaptiveWatermarkFill 及私有辅助方法 ====================

cv::Rect Image::CalculateValidRegion(const cv::Size &image_size,
                                     cv::Rect region) {
  if (region.width < 0) {
    region.x += region.width;
    region.width = -region.width;
  }
  if (region.height < 0) {
    region.y += region.height;
    region.height = -region.height;
  }

  cv::Rect image_boundary(0, 0, image_size.width, image_size.height);
  cv::Rect valid = region & image_boundary;

  if (valid.width <= 0 || valid.height <= 0) {
    return cv::Rect(0, 0, 0, 0);
  }

  return valid;
}

cv::Size Image::CalculateAspectRatioSize(const cv::Size &src_size,
                                         const cv::Size &dst_size) {
  if (src_size.area() <= 0 || dst_size.area() <= 0) {
    return cv::Size(0, 0);
  }

  double width_ratio = dst_size.width / static_cast<double>(src_size.width);
  double height_ratio = dst_size.height / static_cast<double>(src_size.height);
  double min_ratio = std::min(width_ratio, height_ratio);

  cv::Size target_size(cvRound(src_size.width * min_ratio),
                       cvRound(src_size.height * min_ratio));

  target_size.width = std::max(target_size.width, 1);
  target_size.height = std::max(target_size.height, 1);

  return target_size;
}

void Image::AlphaBlend(const cv::Mat &background, const cv::Mat &foreground,
                       cv::Mat &dst, double alpha) {
  cv::addWeighted(background, 1.0 - alpha, foreground, alpha, 0.0, dst);
}

int Image::AdaptiveWatermarkFill(cv::Mat &dest, const cv::Mat &logo,
                                 cv::Rect region, int interpolation,
                                 double alpha) {
  if (dest.empty() || logo.empty()) {
    return kImageEmptyInput;
  }

  if (dest.depth() != CV_8U || logo.depth() != CV_8U) {
    return kImageTypeMismatch;
  }

  if (alpha < 0.0 || alpha > 1.0) {
    return kImageInvalidParam;
  }

  cv::Rect valid_region = CalculateValidRegion(dest.size(), region);
  if (valid_region.area() <= 0) {
    return kImageInvalidRegion;
  }

  cv::Size target_size =
      CalculateAspectRatioSize(logo.size(), valid_region.size());
  if (target_size.area() <= 0) {
    return kImageInvalidRegion;
  }

  cv::Mat scaled_logo;
  cv::resize(logo, scaled_logo, target_size, 0, 0, interpolation);
  if (scaled_logo.size() != valid_region.size()) {
    cv::resize(scaled_logo, scaled_logo, valid_region.size());
  }

  cv::Mat dest_roi = dest(valid_region);
  if (scaled_logo.channels() != dest_roi.channels()) {
    if (dest_roi.channels() == 4) {
      cv::cvtColor(scaled_logo, scaled_logo, cv::COLOR_BGR2BGRA);
    } else if (dest_roi.channels() == 3) {
      if (scaled_logo.channels() == 4) {
        cv::cvtColor(scaled_logo, scaled_logo, cv::COLOR_BGRA2BGR);
      } else if (scaled_logo.channels() == 1) {
        cv::cvtColor(scaled_logo, scaled_logo, cv::COLOR_GRAY2BGR);
      }
    } else if (dest_roi.channels() == 1) {
      cv::cvtColor(scaled_logo, scaled_logo, cv::COLOR_BGR2GRAY);
    }
  }

  cv::Mat blended_roi;
  if (logo.channels() == 4 || dest.channels() == 4) {
    AlphaBlend(dest_roi, scaled_logo, blended_roi, 1.0 - alpha);
  } else {
    cv::addWeighted(dest_roi, 1.0 - alpha, scaled_logo, alpha, 0.0,
                    blended_roi);
  }

  blended_roi.copyTo(dest_roi);
  return 0;
}

}  // namespace kcv
}  // namespace kingfisher
