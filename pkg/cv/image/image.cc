#include "image.h"

#include <Magick++.h>
#include <magick/api.h>

#include "wrap.func.h"
using namespace Magick;
using namespace MagickLib;

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

static int ImageToMat(Magick::Image &image, ColorSpace targetColorSpace,
                      ::cv::Mat &matOutput) {
  if (!image.isValid()) {
    return -1;
  }
  int w = image.columns();
  int h = image.rows();

  image.colorSpace(image.matte() ? Magick::TransparentColorspace
                                 : Magick::RGBColorspace);
  switch (targetColorSpace) {
    case BGRColorSpace:
    case BGRAColorSpace:
      if (image.matte()) {
        matOutput = ::cv::Mat(h, w, CV_8UC4);
        image.write(0, 0, w, h, "BGRA", Magick::CharPixel, matOutput.data);
      } else {
        matOutput = ::cv::Mat(h, w, CV_8UC3);
        image.write(0, 0, w, h, "BGR", Magick::CharPixel, matOutput.data);
      }
      break;
    case GRAYColorSpace:
    case GRAYAColorSpace:
      if (image.matte()) {
        image.type(Magick::GrayscaleMatteType);
        image.write(0, 0, w, h, "BGRA", Magick::CharPixel, matOutput.data);
      } else {
        image.type(Magick::GrayscaleType);
        image.write(0, 0, w, h, "BGR", Magick::CharPixel, matOutput.data);
      }
      break;
    default:
      /*
      if (image.matte()) {
        // use img.channels() for imagemagick
        mat = cv::Mat(h, w, CV_8UC4);
      } else {
        mat = cv::Mat(h, w, CV_8UC3);
      }
      image.write(0, 0, w, h, map, Magick::CharPixel,  matOutput.data);
      */
      return -1;
  }

  matOutput = ::cv::Mat(h, w, CV_8UC3);
  image.write(0, 0, w, h, "BGR", Magick::CharPixel, matOutput.data);
  return 0;
}

// https://github.com/rodlie/cyan/blob/3382ed70237861e178de5e08ea8be1bf4475bff7/src/FXX.cpp
static int imageRead(const std::string &imageData, Magick::Image &imageOutput) {
  auto ret = WrapFuncT([&]() {
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
    WrapFuncT([&]() { image.autoOrient(); });
  }

  return ImageToMat(image, targetColorSpace, matOutput);
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

int Image::GlobalRelease() {
  DestroyMagick();
  return 0;
}

int Image::RotateImage(const std::string &imageData, double degree,
                       ::cv::Mat &matOutput) {
  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    return ret;
  }

  if (degree) {
    ret = WrapFuncT([&]() { image.rotate(degree); });
    if (ret != 0) {
      return ret;
    }
  }

  return ImageToMat(image, matOutput);
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
  // image.write(0, 0, width, height, "BGR", Magick::CharPixel, matOutput.data);

  // return 0;
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
// https://github.com/AndreMouche/GraphicsStudy/blob/master/GraphicsMagicUsage/water_mark_txt.cpp
int Image::AnnotateImage(const std::string &imageData, const std::string &text,
                         const Point &point, ::cv::Mat &matOutput) {
  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    return ret;
  }

  // DrawContext drawContext;
  ret = WrapFuncT([&]() {
  /*DrawContext*/

#if 0
    // drawContext = DrawAllocateContext((DrawInfo *)nullptr, &image);
    drawContext = DrawAllocateContext((DrawInfo *)nullptr, image.image());
    DrawSetFillColorString(drawContext, "red");
    DrawAnnotation(drawContext, point.x, point.y,
                   (const unsigned char *)text.c_str());

    DrawSetTextAntialias(drawContext, 1);
    // DrawSetFont(drawContext, FONT_DEFAULT);
    // DrawSetFontSize(drawContext, font_pointsize);
    DrawSetGravity(drawContext, CenterGravity);
    DrawSetTextEncoding(drawContext, "UTF-8");

    DrawRender(drawContext);
    // image.write("./11111.jpg");
    // DrawDestroyContext(drawContext);
#endif
    // image.annotate(text, Magick::Geometry(100, 100, point.x, point.y));
    // image.fillColor("blue");
    // image.font("Helvetica");
    // image.fontPointsize(14);
    image.annotate("Goodbye cruel world!", "+150+20");
    // Magick::Geometry(rect.width, rect.height, rect.x, rect.y));
  });
  if (ret != 0) {
    return ret;
  }
  return ImageToMat(image, matOutput);
}
#endif

}  // namespace kcv
}  // namespace kingfisher