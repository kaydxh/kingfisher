#include "image.h"

#include "Magick++.h"

namespace kingfisher {
namespace cv {

int Image::GlobalInit() {
  Magick::InitializeMagick(nullptr);
  return 0;
}

int Image::DecodeImage(const std::string &imageData,
                       ColorSpace targetColorSpace, ::cv::Mat &matOutput) {
  Magick::Image image;
  try {
    Magick::Blob blob((void *)imageData.data(), imageData.length());
    image.read(blob);
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

  int rows = image.rows();
  int columns = image.columns();
  if (rows <= 0 || columns <= 0) {
    return -1;
  }

  return ConvertImage(image, targetColorSpace, matOutput);
}

int Image::ConvertImage(Magick::Image &image, ColorSpace targetColorSpace,
                        ::cv::Mat &matOutput) {
  int w = image.columns();
  int h = image.rows();

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

}  // namespace cv
}  // namespace kingfisher
