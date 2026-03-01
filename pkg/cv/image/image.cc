#include "image.h"

#include <Magick++.h>
#include <magick/api.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>

#include "cv/image/image.pb.h"
#include "cv/types/types.h"
#include "log/config.h"
#include "opencv2/highgui/highgui.hpp"
#include "wrap.func.h"

using namespace Magick;
using namespace MagickLib;

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

// 其他需要兼容的宏...
#endif

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
    // use img.channels() for imagemagick
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

// https://github.com/rodlie/cyan/blob/3382ed70237861e178de5e08ea8be1bf4475bff7/src/FXX.cpp
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

static int ConvertImage(Magick::Image &image, ColorSpace targetColorSpace,
                        bool autoOrient, ::cv::Mat &matOutput) {
  if (!image.isValid()) {
    return -1;
  }

  if (autoOrient) {
    WrapMagickFuncT([&]() { image.autoOrient(); });
  }

  return ImageToMat(image, targetColorSpace, matOutput);
}

int Image::GlobalInit() {
  Magick::InitializeMagick(nullptr);
  return 0;
}

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
    return -1;
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
    return -1;
  }
  std::string content{std::istreambuf_iterator<char>(stream), {}};
  return kingfisher::kcv::Image::DecodeImage(content, opts, matOutput);
}

int Image::GlobalRelease() {
  DestroyMagick();
  return 0;
}

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
  // OpenCV 的 getRotationMatrix2D 使用逆时针为正方向，
  // 而外部传入的 angle 为顺时针角度，因此取负值
  cv::Mat rot = cv::getRotationMatrix2D(center, -angle, 1.0);

  // 计算旋转后的包围框大小，确保图像内容不被裁剪
  cv::Rect bbox = cv::RotatedRect(cv::Point2f(), matInput.size(), static_cast<float>(angle)).boundingRect();

  // 调整旋转矩阵的平移分量，使旋转后的图像居中在新画布上
  rot.at<double>(0, 2) += bbox.width / 2.0 - matInput.cols / 2.0;
  rot.at<double>(1, 2) += bbox.height / 2.0 - matInput.rows / 2.0;

  cv::warpAffine(matInput, matOutput, rot, bbox.size());
  LOG(INFO) << "rotate origin image by arbitrary angle=" << angle
            << ", output size=" << matOutput.cols << "x" << matOutput.rows;
  return 0;
}

// ==================== 3.1 图像编码/导出到内存 ====================

int Image::EncodeImage(const cv::Mat &mat, const std::string &format,
                       int quality, std::string &output) {
  if (mat.empty()) {
    LOG(ERROR) << "EncodeImage: input mat is empty";
    return -1;
  }

  std::vector<uchar> buf;
  std::vector<int> params;

  // 根据格式设置编码参数
  std::string fmt = format;
  // 确保格式以 '.' 开头
  if (!fmt.empty() && fmt[0] != '.') {
    fmt = "." + fmt;
  }

  // 统一转为小写
  std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);

  if (fmt == ".jpg" || fmt == ".jpeg") {
    params.push_back(cv::IMWRITE_JPEG_QUALITY);
    params.push_back(std::max(0, std::min(100, quality)));
  } else if (fmt == ".png") {
    params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    // PNG compression level: 0-9，quality 参数映射为压缩级别
    int compression = quality > 0 ? std::max(0, std::min(9, quality / 11)) : 3;
    params.push_back(compression);
#ifdef IMWRITE_WEBP_QUALITY
  } else if (fmt == ".webp") {
    params.push_back(cv::IMWRITE_WEBP_QUALITY);
    params.push_back(std::max(1, std::min(100, quality)));
#endif
  }
  // .bmp 等格式无需额外参数

  auto ret = WrapOpencvFuncT([&]() {
    if (!cv::imencode(fmt, mat, buf, params)) {
      throw cv::Exception(0, "imencode failed for format: " + fmt,
                          "EncodeImage", __FILE__, __LINE__);
    }
  });
  if (ret != 0) {
    LOG(ERROR) << "EncodeImage: failed to encode image to format: " << fmt;
    return -1;
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

  // 如果是 PNG 且指定了 compression，优先使用 compression
  if (!format.empty()) {
    std::string fmt = format;
    std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);
    if ((fmt == "png" || fmt == ".png") && opts.compression() > 0) {
      // 对于 PNG，将 compression 参数传递给 quality
      quality = opts.compression() * 11;  // 映射回 0-99 范围
    }
  }

  return EncodeImage(mat, format, quality, output);
}

// ==================== 3.2 格式转换 ====================

int Image::ConvertFormat(const std::string &inputData,
                         const std::string &targetFormat, int quality,
                         std::string &output) {
  if (inputData.empty()) {
    LOG(ERROR) << "ConvertFormat: input data is empty";
    return -1;
  }

  // 先解码
  cv::Mat mat;
  DecodeOptions opts;
  opts.set_targetcolorspace(BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = DecodeImage(inputData, opts, mat);
  if (ret != 0) {
    LOG(ERROR) << "ConvertFormat: failed to decode input image";
    return ret;
  }

  // 再编码为目标格式
  return EncodeImage(mat, targetFormat, quality, output);
}

// ==================== 3.3 颜色空间转换 ====================

/**
 * @brief 获取 OpenCV 颜色转换码
 * @return 转换码，-1 表示不支持
 */
static int GetCvtColorCode(ColorSpace from, ColorSpace to) {
  // BGR -> 其他
  if (from == BGRColorSpace && to == GRAYColorSpace) return cv::COLOR_BGR2GRAY;
  if (from == BGRColorSpace && to == BGRAColorSpace) return cv::COLOR_BGR2BGRA;
  if (from == BGRColorSpace && to == RGBColorSpace) return cv::COLOR_BGR2RGB;
  if (from == BGRColorSpace && to == RGBAColorSpace) return cv::COLOR_BGR2RGBA;

  // BGRA -> 其他
  if (from == BGRAColorSpace && to == BGRColorSpace) return cv::COLOR_BGRA2BGR;
  if (from == BGRAColorSpace && to == GRAYColorSpace) return cv::COLOR_BGRA2GRAY;
  if (from == BGRAColorSpace && to == RGBColorSpace) return cv::COLOR_BGRA2RGB;
  if (from == BGRAColorSpace && to == RGBAColorSpace) return cv::COLOR_BGRA2RGBA;

  // GRAY -> 其他
  if (from == GRAYColorSpace && to == BGRColorSpace) return cv::COLOR_GRAY2BGR;
  if (from == GRAYColorSpace && to == BGRAColorSpace) return cv::COLOR_GRAY2BGRA;
  if (from == GRAYColorSpace && to == RGBColorSpace) return cv::COLOR_GRAY2RGB;

  // RGB -> 其他
  if (from == RGBColorSpace && to == BGRColorSpace) return cv::COLOR_RGB2BGR;
  if (from == RGBColorSpace && to == GRAYColorSpace) return cv::COLOR_RGB2GRAY;
  if (from == RGBColorSpace && to == BGRAColorSpace) return cv::COLOR_RGB2BGRA;

  // RGBA -> 其他
  if (from == RGBAColorSpace && to == BGRColorSpace) return cv::COLOR_RGBA2BGR;
  if (from == RGBAColorSpace && to == BGRAColorSpace) return cv::COLOR_RGBA2BGRA;

  return -1;  // 不支持的转换
}

int Image::ConvertColorSpace(const cv::Mat &input, ColorSpace from,
                             ColorSpace to, cv::Mat &output) {
  if (input.empty()) {
    LOG(ERROR) << "ConvertColorSpace: input mat is empty";
    return -1;
  }

  if (from == to) {
    output = input.clone();
    return 0;
  }

  int code = GetCvtColorCode(from, to);
  if (code < 0) {
    LOG(ERROR) << "ConvertColorSpace: unsupported conversion from "
               << from << " to " << to;
    return -1;
  }

  auto ret = WrapOpencvFuncT([&]() {
    cv::cvtColor(input, output, code);
  });
  if (ret != 0) {
    LOG(ERROR) << "ConvertColorSpace: cv::cvtColor failed";
    return -1;
  }

  LOG(INFO) << "ConvertColorSpace: converted from " << from << " to " << to;
  return 0;
}

// https://github.com/RyanFu/old_rr_code/blob/a6d3dddb50422f987a97efaba215950d404b0d36/topcc/upload_cwf/imagehelper.cpp
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
  // matOutput = ::cv::Mat(height, width, CV_8UC3);
  // image.write(0, 0, width, height, "BGR", Magick::CharPixel,
  // matOutput.data);

  // return 0;
}

// ==================== 3.7 ResizeImage (Mat 版本) ====================

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

// ==================== 3.7 CropImage (Mat 版本) ====================

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

// ==================== 3.7 CenterCropImage ====================

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

  // 限制裁剪尺寸不超过图像尺寸
  int cropW = std::min(width, matInput.cols);
  int cropH = std::min(height, matInput.rows);

  // 计算中心裁剪区域
  int x = (matInput.cols - cropW) / 2;
  int y = (matInput.rows - cropH) / 2;

  cv::Rect cropRect(x, y, cropW, cropH);
  matOutput = matInput(cropRect).clone();

  LOG(INFO) << "CenterCropImage: center cropped to " << cropW << "x" << cropH;
  return 0;
}

// ==================== 3.5 FlipImage ====================

int Image::FlipImage(const cv::Mat &input, FlipMode mode, cv::Mat &output) {
  if (input.empty()) {
    LOG(ERROR) << "FlipImage: input mat is empty";
    return -1;
  }

  // OpenCV flip 代码:
  // flipCode > 0: 水平翻转（沿Y轴）
  // flipCode == 0: 垂直翻转（沿X轴）
  // flipCode < 0: 同时翻转
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

// ==================== 3.4 图像拼接/合成 ====================

int Image::HConcat(const std::vector<cv::Mat> &images, cv::Mat &output) {
  if (images.empty()) {
    LOG(ERROR) << "HConcat: input images list is empty";
    return -1;
  }
  if (images.size() == 1) {
    output = images[0].clone();
    return 0;
  }

  // 检查所有图像的类型和通道数是否一致
  int type = images[0].type();
  for (size_t i = 1; i < images.size(); ++i) {
    if (images[i].empty()) {
      LOG(ERROR) << "HConcat: image at index " << i << " is empty";
      return -1;
    }
    if (images[i].type() != type) {
      LOG(ERROR) << "HConcat: image type mismatch at index " << i;
      return -1;
    }
  }

  // 找到最大高度，将所有图像缩放到相同高度
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
    return -1;
  }

  LOG(INFO) << "HConcat: concatenated " << images.size()
            << " images horizontally, output size=" << output.cols
            << "x" << output.rows;
  return 0;
}

int Image::VConcat(const std::vector<cv::Mat> &images, cv::Mat &output) {
  if (images.empty()) {
    LOG(ERROR) << "VConcat: input images list is empty";
    return -1;
  }
  if (images.size() == 1) {
    output = images[0].clone();
    return 0;
  }

  // 检查所有图像的类型和通道数是否一致
  int type = images[0].type();
  for (size_t i = 1; i < images.size(); ++i) {
    if (images[i].empty()) {
      LOG(ERROR) << "VConcat: image at index " << i << " is empty";
      return -1;
    }
    if (images[i].type() != type) {
      LOG(ERROR) << "VConcat: image type mismatch at index " << i;
      return -1;
    }
  }

  // 找到最大宽度，将所有图像缩放到相同宽度
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
    return -1;
  }

  LOG(INFO) << "VConcat: concatenated " << images.size()
            << " images vertically, output size=" << output.cols
            << "x" << output.rows;
  return 0;
}

int Image::Overlay(cv::Mat &dest, const cv::Mat &src, int x, int y,
                   double alpha) {
  if (dest.empty() || src.empty()) {
    LOG(ERROR) << "Overlay: dest or src mat is empty";
    return -1;
  }
  if (alpha < 0.0 || alpha > 1.0) {
    LOG(ERROR) << "Overlay: alpha value out of range [0, 1]: " << alpha;
    return -1;
  }

  // 计算实际叠加区域（处理越界）
  int srcX = 0, srcY = 0;
  int dstX = x, dstY = y;
  int overlapW = src.cols, overlapH = src.rows;

  // 处理负坐标
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

  // 处理超出右边界
  if (dstX + overlapW > dest.cols) {
    overlapW = dest.cols - dstX;
  }
  // 处理超出下边界
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

  // 通道对齐
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

  // Alpha 混合
  cv::Mat blended;
  cv::addWeighted(destRegion, 1.0 - alpha, srcAligned, alpha, 0.0, blended);
  blended.copyTo(destRegion);

  LOG(INFO) << "Overlay: overlaid at (" << x << ", " << y
            << "), overlap=" << overlapW << "x" << overlapH
            << ", alpha=" << alpha;
  return 0;
}

// ==================== 3.6 文字标注 ====================

int Image::AnnotateImage(cv::Mat &image, const std::string &text,
                         const cv::Point &position,
                         const AnnotateOptions &opts) {
  if (image.empty()) {
    LOG(ERROR) << "AnnotateImage: input image is empty";
    return -1;
  }
  if (text.empty()) {
    LOG(WARNING) << "AnnotateImage: text is empty, nothing to draw";
    return 0;
  }

  // 构建颜色（BGR格式）
  cv::Scalar color(opts.color_b(), opts.color_g(), opts.color_r());

  // 字体设置
  double fontSize = opts.font_size() > 0 ? opts.font_size() : 1.0;
  int thickness = opts.thickness() > 0 ? opts.thickness() : 1;
  int fontFace = cv::FONT_HERSHEY_SIMPLEX;
  if (opts.anti_alias()) {
    fontFace |= cv::FONT_ITALIC;  // OpenCV 中抗锯齿通过 LINE_AA 控制
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
    return -1;
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

// ==================== 4.2 缩略图生成 ====================

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

  // 如果图像已经小于 maxDimension，直接返回
  if (w <= maxDimension && h <= maxDimension) {
    matOutput = matInput.clone();
    LOG(INFO) << "Thumbnail: image already within max dimension, no resize needed";
    return 0;
  }

  // 计算缩放比例，以长边为准
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

// ==================== 4.4 基础图像滤镜 ====================

int Image::GaussianBlur(const cv::Mat &input, cv::Mat &output,
                        int kernelSize, double sigma) {
  if (input.empty()) {
    LOG(ERROR) << "GaussianBlur: input mat is empty";
    return -1;
  }
  // 确保 kernelSize 为正奇数
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

int Image::Sharpen(const cv::Mat &input, cv::Mat &output,
                   double amount, int kernelSize) {
  if (input.empty()) {
    LOG(ERROR) << "Sharpen: input mat is empty";
    return -1;
  }
  if (kernelSize <= 0) kernelSize = 3;
  if (kernelSize % 2 == 0) kernelSize += 1;

  // USM 锐化：先进行高斯模糊，再用原图减去模糊图像，最后叠加到原图
  cv::Mat blurred;
  auto ret = WrapOpencvFuncT([&]() {
    cv::GaussianBlur(input, blurred, cv::Size(kernelSize, kernelSize), 0);
    // output = input + amount * (input - blurred)
    // 即 output = input * (1 + amount) - blurred * amount
    cv::addWeighted(input, 1.0 + amount, blurred, -amount, 0, output);
  });
  if (ret != 0) {
    LOG(ERROR) << "Sharpen: USM sharpen failed";
    return -1;
  }

  LOG(INFO) << "Sharpen: amount=" << amount << ", kernelSize=" << kernelSize;
  return 0;
}

int Image::AdjustBrightnessContrast(const cv::Mat &input, cv::Mat &output,
                                    double brightness, double contrast) {
  if (input.empty()) {
    LOG(ERROR) << "AdjustBrightnessContrast: input mat is empty";
    return -1;
  }

  // output(x,y) = contrast * input(x,y) + brightness
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

// ==================== 4.3 图像相似度计算 ====================

int Image::ComputeImageHash(const cv::Mat &input, HashType hashType,
                            uint64_t &hashValue) {
  if (input.empty()) {
    LOG(ERROR) << "ComputeImageHash: input mat is empty";
    return -1;
  }

  cv::Mat gray;
  if (input.channels() > 1) {
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
  } else {
    gray = input;
  }

  hashValue = 0;

  switch (hashType) {
    case HASH_AVERAGE: {
      // 平均哈希 (aHash)：缩放到 8x8，计算像素平均值，大于平均值为 1
      cv::Mat resized;
      cv::resize(gray, resized, cv::Size(8, 8), 0, 0, cv::INTER_AREA);
      resized.convertTo(resized, CV_64F);
      double mean = cv::mean(resized)[0];
      for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
          if (resized.at<double>(i, j) > mean) {
            hashValue |= (1ULL << (i * 8 + j));
          }
        }
      }
      break;
    }
    case HASH_PERCEPTUAL: {
      // 感知哈希 (pHash)：缩放到 32x32，DCT 变换，取左上 8x8 低频分量
      cv::Mat resized;
      cv::resize(gray, resized, cv::Size(32, 32), 0, 0, cv::INTER_AREA);
      resized.convertTo(resized, CV_64F);

      cv::Mat dctResult;
      cv::dct(resized, dctResult);

      // 取左上 8x8 的 DCT 系数（不包含 DC 分量）
      cv::Mat dctBlock = dctResult(cv::Rect(0, 0, 8, 8));
      double mean = (cv::sum(dctBlock)[0] - dctBlock.at<double>(0, 0)) / 63.0;

      for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
          if (i == 0 && j == 0) continue;  // 跳过 DC 分量
          if (dctBlock.at<double>(i, j) > mean) {
            hashValue |= (1ULL << (i * 8 + j));
          }
        }
      }
      break;
    }
    case HASH_DIFFERENCE: {
      // 差异哈希 (dHash)：缩放到 9x8，比较相邻像素
      cv::Mat resized;
      cv::resize(gray, resized, cv::Size(9, 8), 0, 0, cv::INTER_AREA);
      resized.convertTo(resized, CV_64F);

      for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
          if (resized.at<double>(i, j) < resized.at<double>(i, j + 1)) {
            hashValue |= (1ULL << (i * 8 + j));
          }
        }
      }
      break;
    }
    default:
      LOG(ERROR) << "ComputeImageHash: unsupported hash type: " << hashType;
      return -1;
  }

  LOG(INFO) << "ComputeImageHash: computed hash type=" << hashType
            << ", value=0x" << std::hex << hashValue << std::dec;
  return 0;
}

int Image::HammingDistance(uint64_t hash1, uint64_t hash2) {
  uint64_t diff = hash1 ^ hash2;
  int distance = 0;
  while (diff) {
    distance += diff & 1;
    diff >>= 1;
  }
  return distance;
}

double Image::ComputeSSIM(const cv::Mat &img1, const cv::Mat &img2) {
  if (img1.empty() || img2.empty()) {
    LOG(ERROR) << "ComputeSSIM: input image is empty";
    return -1.0;
  }
  if (img1.size() != img2.size() || img1.type() != img2.type()) {
    LOG(ERROR) << "ComputeSSIM: images must have same size and type";
    return -1.0;
  }

  // 转换为灰度图
  cv::Mat gray1, gray2;
  if (img1.channels() > 1) {
    cv::cvtColor(img1, gray1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(img2, gray2, cv::COLOR_BGR2GRAY);
  } else {
    gray1 = img1;
    gray2 = img2;
  }

  gray1.convertTo(gray1, CV_64F);
  gray2.convertTo(gray2, CV_64F);

  // SSIM 常量
  double C1 = 6.5025;    // (0.01 * 255)^2
  double C2 = 58.5225;   // (0.03 * 255)^2

  cv::Mat mu1, mu2;
  cv::GaussianBlur(gray1, mu1, cv::Size(11, 11), 1.5);
  cv::GaussianBlur(gray2, mu2, cv::Size(11, 11), 1.5);

  cv::Mat mu1_sq = mu1.mul(mu1);
  cv::Mat mu2_sq = mu2.mul(mu2);
  cv::Mat mu1_mu2 = mu1.mul(mu2);

  cv::Mat sigma1_sq, sigma2_sq, sigma12;
  cv::GaussianBlur(gray1.mul(gray1), sigma1_sq, cv::Size(11, 11), 1.5);
  sigma1_sq -= mu1_sq;

  cv::GaussianBlur(gray2.mul(gray2), sigma2_sq, cv::Size(11, 11), 1.5);
  sigma2_sq -= mu2_sq;

  cv::GaussianBlur(gray1.mul(gray2), sigma12, cv::Size(11, 11), 1.5);
  sigma12 -= mu1_mu2;

  // SSIM 公式
  cv::Mat numerator, denominator;
  numerator = (2 * mu1_mu2 + C1).mul(2 * sigma12 + C2);
  denominator = (mu1_sq + mu2_sq + C1).mul(sigma1_sq + sigma2_sq + C2);

  cv::Mat ssim_map;
  cv::divide(numerator, denominator, ssim_map);

  double ssim = cv::mean(ssim_map)[0];
  LOG(INFO) << "ComputeSSIM: SSIM=" << ssim;
  return ssim;
}

double Image::CompareHistogram(const cv::Mat &img1, const cv::Mat &img2,
                               CompareMethod method) {
  if (img1.empty() || img2.empty()) {
    LOG(ERROR) << "CompareHistogram: input image is empty";
    return -1.0;
  }

  // 转换到 HSV 色彩空间
  cv::Mat hsv1, hsv2;
  if (img1.channels() > 1) {
    cv::cvtColor(img1, hsv1, cv::COLOR_BGR2HSV);
  } else {
    hsv1 = img1;
  }
  if (img2.channels() > 1) {
    cv::cvtColor(img2, hsv2, cv::COLOR_BGR2HSV);
  } else {
    hsv2 = img2;
  }

  // 计算直方图
  int hBins = 50, sBins = 60;
  int histSize[] = {hBins, sBins};
  float hRanges[] = {0, 180};
  float sRanges[] = {0, 256};
  const float *ranges[] = {hRanges, sRanges};
  int channels[] = {0, 1};

  cv::Mat hist1, hist2;
  if (hsv1.channels() >= 2) {
    cv::calcHist(&hsv1, 1, channels, cv::Mat(), hist1, 2, histSize, ranges, true, false);
    cv::normalize(hist1, hist1, 0, 1, cv::NORM_MINMAX);

    cv::calcHist(&hsv2, 1, channels, cv::Mat(), hist2, 2, histSize, ranges, true, false);
    cv::normalize(hist2, hist2, 0, 1, cv::NORM_MINMAX);
  } else {
    // 单通道直方图
    int histSizeSingle[] = {256};
    float range[] = {0, 256};
    const float *rangesSingle[] = {range};
    int channel[] = {0};
    cv::calcHist(&hsv1, 1, channel, cv::Mat(), hist1, 1, histSizeSingle, rangesSingle, true, false);
    cv::normalize(hist1, hist1, 0, 1, cv::NORM_MINMAX);
    cv::calcHist(&hsv2, 1, channel, cv::Mat(), hist2, 1, histSizeSingle, rangesSingle, true, false);
    cv::normalize(hist2, hist2, 0, 1, cv::NORM_MINMAX);
  }

  // 映射 CompareMethod 到 OpenCV 比较方法
  int cvMethod;
  switch (method) {
    case COMPARE_CORRELATION:
#if CV_MAJOR_VERSION >= 4
      cvMethod = cv::HISTCMP_CORREL;
#else
      cvMethod = CV_COMP_CORREL;
#endif
      break;
    case COMPARE_CHI_SQUARE:
#if CV_MAJOR_VERSION >= 4
      cvMethod = cv::HISTCMP_CHISQR;
#else
      cvMethod = CV_COMP_CHISQR;
#endif
      break;
    case COMPARE_INTERSECTION:
#if CV_MAJOR_VERSION >= 4
      cvMethod = cv::HISTCMP_INTERSECT;
#else
      cvMethod = CV_COMP_INTERSECT;
#endif
      break;
    case COMPARE_BHATTACHARYYA:
#if CV_MAJOR_VERSION >= 4
      cvMethod = cv::HISTCMP_BHATTACHARYYA;
#else
      cvMethod = CV_COMP_BHATTACHARYYA;
#endif
      break;
    default:
#if CV_MAJOR_VERSION >= 4
      cvMethod = cv::HISTCMP_CORREL;
#else
      cvMethod = CV_COMP_CORREL;
#endif
      break;
  }

  double result = cv::compareHist(hist1, hist2, cvMethod);
  LOG(INFO) << "CompareHistogram: method=" << method << ", result=" << result;
  return result;
}

// ==================== 4.5 EXIF 信息读取 ====================

int Image::ReadExifInfo(const std::string &imageData, ExifInfo &exifInfo) {
  if (imageData.empty()) {
    LOG(ERROR) << "ReadExifInfo: input data is empty";
    return -1;
  }

  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    LOG(ERROR) << "ReadExifInfo: failed to read image";
    return ret;
  }

  // 基本尺寸信息
  exifInfo.set_image_width(image.columns());
  exifInfo.set_image_height(image.rows());
  exifInfo.set_orientation(
      static_cast<kcv::OrientationType>(image.orientation()));

  // 通过 GraphicsMagick 的 attribute 接口读取 EXIF 信息
  auto readAttr = [&](const std::string &key) -> std::string {
    std::string value;
    WrapMagickFuncT([&]() {
      value = image.attribute(key);
    });
    return value;
  };

  // 相机信息
  std::string make = readAttr("EXIF:Make");
  if (!make.empty()) exifInfo.set_camera_make(make);

  std::string model = readAttr("EXIF:Model");
  if (!model.empty()) exifInfo.set_camera_model(model);

  // 拍摄时间
  std::string datetime = readAttr("EXIF:DateTime");
  if (datetime.empty()) datetime = readAttr("EXIF:DateTimeOriginal");
  if (!datetime.empty()) exifInfo.set_datetime(datetime);

  // 曝光时间
  std::string exposureTime = readAttr("EXIF:ExposureTime");
  if (!exposureTime.empty()) {
    // 曝光时间可能是分数形式如 "1/100"
    size_t slashPos = exposureTime.find('/');
    if (slashPos != std::string::npos) {
      double num = std::stod(exposureTime.substr(0, slashPos));
      double den = std::stod(exposureTime.substr(slashPos + 1));
      if (den > 0) exifInfo.set_exposure_time(num / den);
    } else {
      try { exifInfo.set_exposure_time(std::stod(exposureTime)); }
      catch (...) {}
    }
  }

  // 光圈值
  std::string fNumber = readAttr("EXIF:FNumber");
  if (!fNumber.empty()) {
    size_t slashPos = fNumber.find('/');
    if (slashPos != std::string::npos) {
      double num = std::stod(fNumber.substr(0, slashPos));
      double den = std::stod(fNumber.substr(slashPos + 1));
      if (den > 0) exifInfo.set_f_number(num / den);
    } else {
      try { exifInfo.set_f_number(std::stod(fNumber)); }
      catch (...) {}
    }
  }

  // ISO
  std::string iso = readAttr("EXIF:ISOSpeedRatings");
  if (!iso.empty()) {
    try { exifInfo.set_iso_speed(std::stoi(iso)); }
    catch (...) {}
  }

  // 焦距
  std::string focalLength = readAttr("EXIF:FocalLength");
  if (!focalLength.empty()) {
    size_t slashPos = focalLength.find('/');
    if (slashPos != std::string::npos) {
      double num = std::stod(focalLength.substr(0, slashPos));
      double den = std::stod(focalLength.substr(slashPos + 1));
      if (den > 0) exifInfo.set_focal_length(num / den);
    } else {
      try { exifInfo.set_focal_length(std::stod(focalLength)); }
      catch (...) {}
    }
  }

  // GPS 信息
  std::string gpsLat = readAttr("EXIF:GPSLatitude");
  std::string gpsLon = readAttr("EXIF:GPSLongitude");
  std::string gpsAlt = readAttr("EXIF:GPSAltitude");
  if (!gpsLat.empty()) {
    try { exifInfo.set_gps_latitude(std::stod(gpsLat)); }
    catch (...) {}
  }
  if (!gpsLon.empty()) {
    try { exifInfo.set_gps_longitude(std::stod(gpsLon)); }
    catch (...) {}
  }
  if (!gpsAlt.empty()) {
    size_t slashPos = gpsAlt.find('/');
    if (slashPos != std::string::npos) {
      double num = std::stod(gpsAlt.substr(0, slashPos));
      double den = std::stod(gpsAlt.substr(slashPos + 1));
      if (den > 0) exifInfo.set_gps_altitude(num / den);
    } else {
      try { exifInfo.set_gps_altitude(std::stod(gpsAlt)); }
      catch (...) {}
    }
  }

  // 软件信息
  std::string software = readAttr("EXIF:Software");
  if (!software.empty()) exifInfo.set_software(software);

  // 描述
  std::string description = readAttr("EXIF:ImageDescription");
  if (!description.empty()) exifInfo.set_description(description);

  // 版权
  std::string copyright = readAttr("EXIF:Copyright");
  if (!copyright.empty()) exifInfo.set_copyright(copyright);

  LOG(INFO) << "ReadExifInfo: read EXIF info from image, size="
            << exifInfo.image_width() << "x" << exifInfo.image_height();
  return 0;
}

// ==================== 4.6 批量处理 ====================

int Image::BatchDecodeImages(
    const std::vector<std::string> &imageDatas, const DecodeOptions &opts,
    std::vector<cv::Mat> &matOutputs) {
  if (imageDatas.empty()) {
    LOG(ERROR) << "BatchDecodeImages: input list is empty";
    return -1;
  }

  matOutputs.resize(imageDatas.size());
  int successCount = 0;

  for (size_t i = 0; i < imageDatas.size(); ++i) {
    int ret = DecodeImage(imageDatas[i], opts, matOutputs[i]);
    if (ret == 0) {
      ++successCount;
    } else {
      LOG(WARNING) << "BatchDecodeImages: failed to decode image at index " << i;
    }
  }

  LOG(INFO) << "BatchDecodeImages: decoded " << successCount
            << "/" << imageDatas.size() << " images";
  return successCount > 0 ? successCount : -1;
}

int Image::BatchResizeImages(
    const std::vector<cv::Mat> &inputs, int width, int height,
    bool keepRatio, std::vector<cv::Mat> &outputs) {
  if (inputs.empty()) {
    LOG(ERROR) << "BatchResizeImages: input list is empty";
    return -1;
  }
  if (width <= 0 || height <= 0) {
    LOG(ERROR) << "BatchResizeImages: invalid target size: "
               << width << "x" << height;
    return -1;
  }

  outputs.resize(inputs.size());
  int successCount = 0;

  for (size_t i = 0; i < inputs.size(); ++i) {
    int ret = ResizeImage(inputs[i], width, height, keepRatio, outputs[i]);
    if (ret == 0) {
      ++successCount;
    } else {
      LOG(WARNING) << "BatchResizeImages: failed to resize image at index " << i;
    }
  }

  LOG(INFO) << "BatchResizeImages: resized " << successCount
            << "/" << inputs.size() << " images to " << width << "x" << height;
  return successCount > 0 ? successCount : -1;
}

// ==================== 4.7 Mat → Magick::Image 反向转换 ====================

int Image::MatToImage(const cv::Mat &mat, Magick::Image &imageOutput) {
  if (mat.empty()) {
    LOG(ERROR) << "MatToImage: input mat is empty";
    return -1;
  }

  std::string map;
  int channels = mat.channels();
  switch (channels) {
    case 1:
      map = "I";  // 单通道灰度
      break;
    case 3:
      map = "BGR";
      break;
    case 4:
      map = "BGRA";
      break;
    default:
      LOG(ERROR) << "MatToImage: unsupported channel count: " << channels;
      return -1;
  }

  // 确保数据是连续的
  cv::Mat continuous;
  if (mat.isContinuous()) {
    continuous = mat;
  } else {
    continuous = mat.clone();
  }

  auto ret = WrapMagickFuncT([&]() {
    imageOutput.read(mat.cols, mat.rows, map, Magick::CharPixel,
                     continuous.data);
    // 设置透明通道
    if (channels == 4) {
      imageOutput.matte(true);
    }
  });
  if (ret != 0) {
    LOG(ERROR) << "MatToImage: Magick::Image::read failed";
    return -1;
  }

  LOG(INFO) << "MatToImage: converted cv::Mat (" << mat.cols << "x" << mat.rows
            << ", " << channels << "ch) to Magick::Image";
  return 0;
}

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

// 旧版 AnnotateImage 实现（已被新的 OpenCV 实现替代）
// 新实现见上方 Image::AnnotateImage(cv::Mat &, ...)

/**
 * @brief 计算有效区域（处理负坐标和越界）
 */
cv::Rect Image::CalculateValidRegion(const cv::Size &image_size,
                                     cv::Rect region) {
  // 处理负坐标（OpenCV的Rect::operator&对负尺寸处理不够健壮）
  if (region.width < 0) {
    region.x += region.width;
    region.width = -region.width;
  }
  if (region.height < 0) {
    region.y += region.height;
    region.height = -region.height;
  }

  // 计算实际可见区域
  cv::Rect image_boundary(0, 0, image_size.width, image_size.height);
  cv::Rect valid = region & image_boundary;

  // 处理完全不可见的情况
  if (valid.width <= 0 || valid.height <= 0) {
    return cv::Rect(0, 0, 0, 0);
  }

  return valid;
}

/**
 * @brief 计算保持纵横比的目标尺寸
 */
cv::Size Image::CalculateAspectRatioSize(const cv::Size &src_size,
                                         const cv::Size &dst_size) {
  if (src_size.area() <= 0 || dst_size.area() <= 0) {
    return cv::Size(0, 0);
  }

  // 计算缩放比例
  double width_ratio = dst_size.width / static_cast<double>(src_size.width);
  double height_ratio = dst_size.height / static_cast<double>(src_size.height);
  double min_ratio = std::min(width_ratio, height_ratio);

  // 计算最大内接矩形
  cv::Size target_size(cvRound(src_size.width * min_ratio),
                       cvRound(src_size.height * min_ratio));

  // 保证最小尺寸为1像素
  target_size.width = std::max(target_size.width, 1);
  target_size.height = std::max(target_size.height, 1);

  return target_size;
}

/**
 * @brief Alpha混合实现（复用之前优化的版本）
 */
void Image::AlphaBlend(const cv::Mat &background, const cv::Mat &foreground,
                       cv::Mat &dst, double alpha) {
  // 此处复用之前实现的AlphaBlend函数
  // 可根据需要替换为实际的优化实现
  cv::addWeighted(background, 1.0 - alpha, foreground, alpha, 0.0, dst);
}

int Image::AdaptiveWatermarkFill(cv::Mat &dest, const cv::Mat &logo,
                                 cv::Rect region, int interpolation,
                                 double alpha) {
  // ==================== 输入验证 ====================
  if (dest.empty() || logo.empty()) {
    return -1;
  }

  if (dest.depth() != CV_8U || logo.depth() != CV_8U) {
    return -1;
  }

  if (alpha < 0.0 || alpha > 1.0) {
    return -1;
  }

  // ==================== 区域规范化 ====================
  // 计算有效区域（处理负坐标和越界情况）
  cv::Rect valid_region = CalculateValidRegion(dest.size(), region);
  if (valid_region.area() <= 0) {
    return -1;
  }

  // ==================== 自适应尺寸计算 ====================
  cv::Size target_size =
      CalculateAspectRatioSize(logo.size(), valid_region.size());
  if (target_size.area() <= 0) {
    return -1;
  }

  // ==================== 水印预处理 ====================
  cv::Mat scaled_logo;
  cv::resize(logo, scaled_logo, target_size, 0, 0, interpolation);
  if (scaled_logo.size() != valid_region.size()) {  // 二次尺寸校验
    cv::resize(scaled_logo, scaled_logo, valid_region.size());
  }

  // ==================== 通道统一化 ====================
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
  // ==================== 核心混合操作 ====================
  if (logo.channels() == 4 || dest.channels() == 4) {
    AlphaBlend(dest_roi, scaled_logo, blended_roi, 1.0 - alpha);
  } else {
    cv::addWeighted(dest_roi, 1.0 - alpha, scaled_logo, alpha, 0.0,
                    blended_roi);
  }

  // ==================== 结果回写 ====================
  blended_roi.copyTo(dest_roi);
  return 0;
}

}  // namespace kcv
}  // namespace kingfisher
