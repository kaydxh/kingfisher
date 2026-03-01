
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

// 内部辅助函数（供 ReadExifInfo 使用）
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

// ==================== ComputeImageHash ====================

int Image::ComputeImageHash(const cv::Mat &input, HashType hashType,
                            uint64_t &hashValue) {
  if (input.empty()) {
    LOG(ERROR) << "ComputeImageHash: input mat is empty";
    return kImageEmptyInput;
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
      cv::Mat resized;
      cv::resize(gray, resized, cv::Size(32, 32), 0, 0, cv::INTER_AREA);
      resized.convertTo(resized, CV_64F);

      cv::Mat dctResult;
      cv::dct(resized, dctResult);

      cv::Mat dctBlock = dctResult(cv::Rect(0, 0, 8, 8));
      double mean = (cv::sum(dctBlock)[0] - dctBlock.at<double>(0, 0)) / 63.0;

      for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
          if (i == 0 && j == 0) continue;
          if (dctBlock.at<double>(i, j) > mean) {
            hashValue |= (1ULL << (i * 8 + j));
          }
        }
      }
      break;
    }
    case HASH_DIFFERENCE: {
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
      return kImageInvalidParam;
  }

  LOG(INFO) << "ComputeImageHash: computed hash type=" << hashType
            << ", value=0x" << std::hex << hashValue << std::dec;
  return 0;
}

// ==================== HammingDistance ====================

int Image::HammingDistance(uint64_t hash1, uint64_t hash2) {
  uint64_t diff = hash1 ^ hash2;
  int distance = 0;
  while (diff) {
    distance += diff & 1;
    diff >>= 1;
  }
  return distance;
}

// ==================== ComputeSSIM ====================

double Image::ComputeSSIM(const cv::Mat &img1, const cv::Mat &img2) {
  if (img1.empty() || img2.empty()) {
    LOG(ERROR) << "ComputeSSIM: input image is empty";
    return static_cast<double>(kImageEmptyInput);
  }
  if (img1.size() != img2.size() || img1.type() != img2.type()) {
    LOG(ERROR) << "ComputeSSIM: images must have same size and type";
    return static_cast<double>(kImageSizeMismatch);
  }

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

  double C1 = 6.5025;
  double C2 = 58.5225;

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

  cv::Mat numerator, denominator;
  numerator = (2 * mu1_mu2 + C1).mul(2 * sigma12 + C2);
  denominator = (mu1_sq + mu2_sq + C1).mul(sigma1_sq + sigma2_sq + C2);

  cv::Mat ssim_map;
  cv::divide(numerator, denominator, ssim_map);

  double ssim = cv::mean(ssim_map)[0];
  LOG(INFO) << "ComputeSSIM: SSIM=" << ssim;
  return ssim;
}

// ==================== CompareHistogram ====================

double Image::CompareHistogram(const cv::Mat &img1, const cv::Mat &img2,
                               CompareMethod method) {
  if (img1.empty() || img2.empty()) {
    LOG(ERROR) << "CompareHistogram: input image is empty";
    return static_cast<double>(kImageEmptyInput);
  }

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
    int histSizeSingle[] = {256};
    float range[] = {0, 256};
    const float *rangesSingle[] = {range};
    int channel[] = {0};
    cv::calcHist(&hsv1, 1, channel, cv::Mat(), hist1, 1, histSizeSingle, rangesSingle, true, false);
    cv::normalize(hist1, hist1, 0, 1, cv::NORM_MINMAX);
    cv::calcHist(&hsv2, 1, channel, cv::Mat(), hist2, 1, histSizeSingle, rangesSingle, true, false);
    cv::normalize(hist2, hist2, 0, 1, cv::NORM_MINMAX);
  }

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

// ==================== ReadExifInfo ====================

int Image::ReadExifInfo(const std::string &imageData, ExifInfo &exifInfo) {
  if (imageData.empty()) {
    LOG(ERROR) << "ReadExifInfo: input data is empty";
    return kImageInvalidData;
  }

  Magick::Image image;
  auto ret = imageRead(imageData, image);
  if (ret != 0) {
    LOG(ERROR) << "ReadExifInfo: failed to read image";
    return ret;
  }

  exifInfo.set_image_width(image.columns());
  exifInfo.set_image_height(image.rows());
  exifInfo.set_orientation(
      static_cast<kcv::OrientationType>(image.orientation()));

  auto readAttr = [&](const std::string &key) -> std::string {
    std::string value;
    WrapMagickFuncT([&]() {
      value = image.attribute(key);
    });
    return value;
  };

  std::string make = readAttr("EXIF:Make");
  if (!make.empty()) exifInfo.set_camera_make(make);

  std::string model = readAttr("EXIF:Model");
  if (!model.empty()) exifInfo.set_camera_model(model);

  std::string datetime = readAttr("EXIF:DateTime");
  if (datetime.empty()) datetime = readAttr("EXIF:DateTimeOriginal");
  if (!datetime.empty()) exifInfo.set_datetime(datetime);

  std::string exposureTime = readAttr("EXIF:ExposureTime");
  if (!exposureTime.empty()) {
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

  std::string iso = readAttr("EXIF:ISOSpeedRatings");
  if (!iso.empty()) {
    try { exifInfo.set_iso_speed(std::stoi(iso)); }
    catch (...) {}
  }

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

  std::string software = readAttr("EXIF:Software");
  if (!software.empty()) exifInfo.set_software(software);

  std::string description = readAttr("EXIF:ImageDescription");
  if (!description.empty()) exifInfo.set_description(description);

  std::string copyright = readAttr("EXIF:Copyright");
  if (!copyright.empty()) exifInfo.set_copyright(copyright);

  LOG(INFO) << "ReadExifInfo: read EXIF info from image, size="
            << exifInfo.image_width() << "x" << exifInfo.image_height();
  return 0;
}

// ==================== BatchDecodeImages ====================

int Image::BatchDecodeImages(
    const std::vector<std::string> &imageDatas, const DecodeOptions &opts,
    std::vector<cv::Mat> &matOutputs) {
  if (imageDatas.empty()) {
    LOG(ERROR) << "BatchDecodeImages: input list is empty";
    return kImageInvalidParam;
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
  return successCount > 0 ? successCount : kImageDecodeError;
}

// ==================== BatchResizeImages ====================

int Image::BatchResizeImages(
    const std::vector<cv::Mat> &inputs, int width, int height,
    bool keepRatio, std::vector<cv::Mat> &outputs) {
  if (inputs.empty()) {
    LOG(ERROR) << "BatchResizeImages: input list is empty";
    return kImageInvalidParam;
  }
  if (width <= 0 || height <= 0) {
    LOG(ERROR) << "BatchResizeImages: invalid target size: "
               << width << "x" << height;
    return kImageInvalidParam;
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
  return successCount > 0 ? successCount : kImageUnsupportedOperation;
}

}  // namespace kcv
}  // namespace kingfisher
