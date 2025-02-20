#ifndef KINGFISHER_PKG_CV_IMAGE_H_
#define KINGFISHER_PKG_CV_IMAGE_H_

#if (CV_MAJOR_VERSION >= 4)
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <opencv2/imgproc/types_c.h>

#include <opencv2/opencv.hpp>
#else
#include <opencv2/opencv.hpp>
#endif

#include "Magick++.h"
#include "image.pb.h"

namespace kingfisher {
namespace kcv {

class Image {
 public:
  static int GlobalInit();
  static int GlobalRelease();

  static int PingImage(ImageInfo &result, const std::string &imageData);

  static int DecodeImageFile(const std::string &imageFile,
                             ::cv::Mat &matOutput);

  static int DecodeImageFile(const std::string &imageFile,
                             const DecodeOptions &opts, ::cv::Mat &matOutput);

  static int DecodeImage(const std::string &imageData,
                         const DecodeOptions &opts, ::cv::Mat &matOutput);

  static int RotateImage(const std::string &imageData, double angle,
                         ::cv::Mat &matOutput);

  static int RotateImage(const ::cv::Mat &matInput, double angle,
                         ::cv::Mat &matOutput);

  static int ResizeImage(const std::string &imageData, int width, int height,
                         bool keepRatio, ::cv::Mat &matOutput);

  static int CropImage(const std::string &imageData, const Rect &rect,
                       ::cv::Mat &matOutput);

  static int WriteImage(const cv::Mat &mat, const std::string &path);

  static int DumpImageFileToBytes(const std::string &imageFile,
                                  const std::string &path);

  static int DumpImageToBytes(const cv::Mat &mat, const std::string &path);
  // static int AnnotateImage(const std::string &imageData,
  //                        const std::string &text, const Rect &rect,
  //                       ::cv::Mat &matOutput);
  // static int ZoomImage(const std::string &imageData, double ratio,
  //                     ::cv::Mat &matOutput);

  /**
 * @brief 将水印自适应填充到目标图像的指定区域
 *
 * @param dest 目标图像（支持BGR/BGRA格式，将被直接修改）
 * @param logo 水印图像（支持GRAY/BGR/BGRA格式）
 * @param region 目标区域（允许负坐标和部分超出边界的情况）
 * @param interpolation 缩放插值方法（默认线性插值）
 * @param alpha 混合透明度（0.0-1.0，1.0表示完全不透明）
 *
 * @throws cv::Exception 当输入无效时抛出异常
 *
 * 实现特性：
 * 1. 智能区域规范化：自动处理负坐标和越界区域
 * 2. 保持纵横比：基于最大内接矩形自动调整
 * 3. 高性能：最小化内存拷贝，使用并行处理
 * 4. 精确混合：支持预乘Alpha的正确处理
 */
  static void AdaptiveWatermarkFill(
      cv::Mat& dest,
      const cv::Mat& logo,
      cv::Rect region,
      int interpolation = cv::INTER_LINEAR,
      double alpha = 1.0);

private:
  static cv::Rect CalculateValidRegion(const cv::Size& image_size, cv::Rect region);
  static cv::Size CalculateAspectRatioSize(const cv::Size& src_size, const cv::Size& dst_size);
  static void AlphaBlend(const cv::Mat& background, const cv::Mat& foreground,
                   cv::Mat& dst, double alpha);
};

}  // namespace kcv
}  // namespace kingfisher

#endif
