#include "image.h"

#include <Magick++.h>
#include <magick/api.h>

#include <algorithm>
#include <fstream>
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
