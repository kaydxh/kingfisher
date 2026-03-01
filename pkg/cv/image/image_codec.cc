
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

// ==================== 内部辅助函数 ====================

static int ImageToMat(Magick::Image &image, ColorSpace targetColorSpace,
                      ::cv::Mat &matOutput) {
  if (!image.isValid()) {
    return kImageInvalidData;
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
    return kImageInvalidData;
  }

  return 0;
}

static int ConvertImage(Magick::Image &image, ColorSpace targetColorSpace,
                        bool autoOrient, ::cv::Mat &matOutput) {
  if (!image.isValid()) {
    return kImageInvalidData;
  }

  if (autoOrient) {
    WrapMagickFuncT([&]() { image.autoOrient(); });
  }

  return ImageToMat(image, targetColorSpace, matOutput);
}

// ==================== GlobalInit / GlobalRelease ====================

int Image::GlobalInit() {
  Magick::InitializeMagick(nullptr);
  return 0;
}

int Image::GlobalRelease() {
  DestroyMagick();
  return 0;
}

// ==================== PingImage ====================

int Image::PingImage(ImageInfo &result, const std::string &imageData) {
  Magick::Image image;
  std::string msg;
  int ret = WrapMagickFuncTWithMsg(msg, [&]() {
    Magick::Blob blob((void *)(imageData.data()), imageData.length());
    image.ping(blob);
  });
  if (ret != 0) {
    return ret;
  }

  result.set_columns(image.columns());
  result.set_rows(image.rows());
  result.set_color_space(static_cast<kcv::ColorSpace>(image.colorSpace()));
  result.set_depth(image.depth());
  result.set_file_name(image.fileName());
  result.set_file_size(image.fileSize());
  result.set_format(image.format());
  result.set_gama(image.gamma());
  result.set_is_valid(image.isValid());
  result.set_line_width(image.lineWidth());
  result.set_magick(image.magick());
  result.set_matte(image.matte());
  result.set_orientation(
      static_cast<kcv::OrientationType>(image.orientation()));

  result.set_quality(image.quality());
  result.set_quantize_colors(image.quantizeColors());
  result.set_tile_name(image.tileName());
  result.set_type(static_cast<kcv::ImageType>(image.type()));
  result.set_magick_warning(msg);

  return 0;
}

// ==================== DecodeImage ====================

int Image::DecodeImage(const std::string &imageData, const DecodeOptions &opts,
                       ::cv::Mat &matOutput) {
  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    return ret;
  }
  return ConvertImage(image, opts.targetcolorspace(), opts.auto_orient(),
                      matOutput);
}

int Image::DecodeImageFile(const std::string &imageFile, ::cv::Mat &matOutput) {
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  if (!stream.is_open()) {
    LOG(ERROR) << "failed to open image file: " << imageFile;
    return kImageFileOpenError;
  }
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  opts.set_auto_orient(true);
  return kingfisher::kcv::Image::DecodeImage(content, opts, matOutput);
}

int Image::DecodeImageFile(const std::string &imageFile,
                           const DecodeOptions &opts, ::cv::Mat &matOutput) {
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  if (!stream.is_open()) {
    LOG(ERROR) << "failed to open image file: " << imageFile;
    return kImageFileOpenError;
  }
  std::string content{std::istreambuf_iterator<char>(stream), {}};
  return kingfisher::kcv::Image::DecodeImage(content, opts, matOutput);
}

// ==================== EncodeImage ====================

int Image::EncodeImage(const cv::Mat &mat, const std::string &format,
                       int quality, std::string &output) {
  if (mat.empty()) {
    LOG(ERROR) << "EncodeImage: input mat is empty";
    return kImageEmptyInput;
  }

  std::vector<uchar> buf;
  std::vector<int> params;

  std::string fmt = format;
  if (!fmt.empty() && fmt[0] != '.') {
    fmt = "." + fmt;
  }

  std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);

  if (fmt == ".jpg" || fmt == ".jpeg") {
    params.push_back(cv::IMWRITE_JPEG_QUALITY);
    params.push_back(std::max(0, std::min(100, quality)));
  } else if (fmt == ".png") {
    params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    int compression = quality > 0 ? std::max(0, std::min(9, quality / 11)) : 3;
    params.push_back(compression);
#ifdef IMWRITE_WEBP_QUALITY
  } else if (fmt == ".webp") {
    params.push_back(cv::IMWRITE_WEBP_QUALITY);
    params.push_back(std::max(1, std::min(100, quality)));
#endif
  }

  auto ret = WrapOpencvFuncT([&]() {
    if (!cv::imencode(fmt, mat, buf, params)) {
      throw cv::Exception(0, "imencode failed for format: " + fmt,
                          "EncodeImage", __FILE__, __LINE__);
    }
  });
  if (ret != 0) {
    LOG(ERROR) << "EncodeImage: failed to encode image to format: " << fmt;
    return kImageEncodeError;
  }

  output.assign(reinterpret_cast<const char *>(buf.data()), buf.size());
  LOG(INFO) << "EncodeImage: encoded image to format=" << fmt
            << ", size=" << output.size() << " bytes";
  return 0;
}

int Image::EncodeImage(const cv::Mat &mat, const EncodeOptions &opts,
                       std::string &output) {
  std::string format = opts.format();
  int quality = opts.quality();

  if (!format.empty()) {
    std::string fmt = format;
    std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);
    if ((fmt == "png" || fmt == ".png") && opts.compression() > 0) {
      quality = opts.compression() * 11;
    }
  }

  return EncodeImage(mat, format, quality, output);
}

// ==================== ConvertFormat ====================

int Image::ConvertFormat(const std::string &inputData,
                         const std::string &targetFormat, int quality,
                         std::string &output) {
  if (inputData.empty()) {
    LOG(ERROR) << "ConvertFormat: input data is empty";
    return kImageInvalidData;
  }

  cv::Mat mat;
  DecodeOptions opts;
  opts.set_targetcolorspace(BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = DecodeImage(inputData, opts, mat);
  if (ret != 0) {
    LOG(ERROR) << "ConvertFormat: failed to decode input image";
    return ret;
  }

  return EncodeImage(mat, targetFormat, quality, output);
}

}  // namespace kcv
}  // namespace kingfisher
