#include "image.h"

#include "Magick++.h"
#include "wrap.func.h"

namespace kingfisher {
namespace kcv {

static int ImageToMat(Magick::Image &image, ::cv::Mat &matOutput) {
  if (!image.isValid()) {
    return -1;
  }
  int w = image.columns();
  int h = image.rows();

  matOutput = ::cv::Mat(h, w, CV_8UC3);
  image.write(0, 0, w, h, "BGR", Magick::CharPixel, matOutput.data);
  return 0;
}

static int imageRead(const std::string &imageData, Magick::Image &imageOutput) {
  auto ret = WrapFuncT([&]() {
    Magick::Blob blob((void *)imageData.data(), imageData.length());
    imageOutput.read(blob);
  });
#if 0
  try {
    Magick::Blob blob((void *)imageData.data(), imageData.length());
    imageOutput.read(blob);
  } catch (Magick::Warning &w) {
    std::cout << "warn: " << w.what() << std::endl;
  } catch (Magick::Error &e) {
    std::cout << "a Magick++ error occurred: " << e.what() << std::endl;
    return -1;
  } catch (...) {
    std::cout << "an unhandled error has occurred; exiting application."
              << std::endl;
    return -1;
  }
#endif
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
  int w = image.columns();
  int h = image.rows();

  if (autoOrient) {
    WrapFuncT([&]() { image.autoOrient(); });
  }

  image.colorSpace(Magick::RGBColorspace);
  switch (targetColorSpace) {
    case BGRColorSpace:
      // image.colorSpace(Magick::CMYKColorspace);
      // image.quantizeColorSpace(Magick::CMYKColorspace);
      // image.type(Magick::GrayscaleMatteType);
      // image.colorSpace(Magick::RGBColorspace);
      matOutput = ::cv::Mat(h, w, CV_8UC3);
      image.write(0, 0, w, h, "BGR", Magick::CharPixel, matOutput.data);
      break;
    case BGRAColorSpace:
      matOutput = ::cv::Mat(h, w, CV_8UC4);
      image.write(0, 0, w, h, "BGRA", Magick::CharPixel, matOutput.data);
      break;
    case GRAYColorSpace:
      image.type(Magick::GrayscaleType);
      matOutput = ::cv::Mat(h, w, CV_8UC3);
      image.write(0, 0, w, h, "BGR", Magick::CharPixel, matOutput.data);
      break;
    case GRAYAColorSpace:
      image.type(Magick::GrayscaleMatteType);
      matOutput = ::cv::Mat(h, w, CV_8UC4);
      image.write(0, 0, w, h, "BGRA", Magick::CharPixel, matOutput.data);
      break;
    default:
      return -1;
  }

  return 0;
}

int Image::GlobalInit() {
  Magick::InitializeMagick(nullptr);
  return 0;
}

int Image::DecodeImage(const std::string &imageData, const DecodeOptions &opts,
                       ::cv::Mat &matOutput) {
  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    return 0;
  }

  return ConvertImage(image, opts.targetcolorspace(), opts.auto_orient(),
                      matOutput);
}

int Image::RotateImage(const std::string &imageData, double degree,
                       ::cv::Mat &matOutput) {
  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    return ret;
  }

  if (degree) {
    image.rotate(degree);
  }

  int w = image.columns();
  int h = image.rows();
  matOutput = ::cv::Mat(h, w, CV_8UC3);
  image.write(0, 0, w, h, "BGR", Magick::CharPixel, matOutput.data);

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

  matOutput = ::cv::Mat(height, width, CV_8UC3);
  image.write(0, 0, width, height, "BGR", Magick::CharPixel, matOutput.data);

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
      rect0 & cv::Rect(rect.x(), rect.y(), rect.height(), rect.width());
  image.crop(Magick::Geometry(intesectRect.width, intesectRect.height,
                              intesectRect.x, intesectRect.y));
  return ImageToMat(image, matOutput);
}

#if 0
int Image::AnnotateImage(const std::string &imageData, const std::string &text,
                         const Rect &rect, ::cv::Mat &matOutput) {
  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    return ret;
  }

  ret = WrapFuncT([&]() {
    image.annotate(text,
                   Magick::Geometry(rect.width, rect.height, rect.x, rect.y));
  });
  if (ret != 0) {
    return ret;
  }
  return ImageToMat(image, matOutput);
}
#endif

}  // namespace kcv
}  // namespace kingfisher
