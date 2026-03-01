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

  // ==================== 编码/导出 ====================

  /**
   * @brief 将 cv::Mat 编码为指定格式的内存 buffer
   *
   * @param mat 输入图像
   * @param format 编码格式，如 ".jpg", ".png", ".webp", ".bmp"
   * @param quality 编码质量（JPEG/WebP: 0-100）
   * @param output 输出的编码数据
   * @return 0 成功，负数失败
   */
  static int EncodeImage(const cv::Mat &mat, const std::string &format,
                         int quality, std::string &output);

  /**
   * @brief 将 cv::Mat 编码为指定格式的内存 buffer（使用 EncodeOptions）
   */
  static int EncodeImage(const cv::Mat &mat, const EncodeOptions &opts,
                         std::string &output);

  // ==================== 格式转换 ====================

  /**
   * @brief 图像格式互转（如 PNG→JPEG、BMP→WebP 等）
   *
   * @param inputData 原始图像字节数据
   * @param targetFormat 目标格式，如 ".jpg", ".png", ".webp"
   * @param quality 编码质量
   * @param output 输出的转换后数据
   * @return 0 成功，负数失败
   */
  static int ConvertFormat(const std::string &inputData,
                           const std::string &targetFormat, int quality,
                           std::string &output);

  // ==================== 颜色空间转换 ====================

  /**
   * @brief 独立的颜色空间转换
   *
   * @param input 输入图像
   * @param from 源颜色空间
   * @param to 目标颜色空间
   * @param output 输出图像
   * @return 0 成功，负数失败
   */
  static int ConvertColorSpace(const cv::Mat &input, ColorSpace from,
                               ColorSpace to, cv::Mat &output);

  // ==================== 旋转 ====================

  static int RotateImage(const std::string &imageData, double angle,
                         ::cv::Mat &matOutput);

  static int RotateImage(const ::cv::Mat &matInput, double angle,
                         ::cv::Mat &matOutput);

  // ==================== 缩放 ====================

  static int ResizeImage(const std::string &imageData, int width, int height,
                         bool keepRatio, ::cv::Mat &matOutput);

  /**
   * @brief 缩放图像（cv::Mat 输入版本）
   *
   * @param matInput 输入图像
   * @param width 目标宽度
   * @param height 目标高度
   * @param keepRatio 是否保持宽高比
   * @param matOutput 输出图像
   * @return 0 成功，负数失败
   */
  static int ResizeImage(const cv::Mat &matInput, int width, int height,
                         bool keepRatio, cv::Mat &matOutput);

  // ==================== 裁剪 ====================

  static int CropImage(const std::string &imageData, const Rect &rect,
                       ::cv::Mat &matOutput);

  /**
   * @brief 裁剪图像（cv::Mat 输入版本）
   *
   * @param matInput 输入图像
   * @param rect 裁剪区域
   * @param matOutput 输出图像
   * @return 0 成功，负数失败
   */
  static int CropImage(const cv::Mat &matInput, const Rect &rect,
                       cv::Mat &matOutput);

  /**
   * @brief 中心裁剪
   *
   * @param matInput 输入图像
   * @param width 裁剪宽度
   * @param height 裁剪高度
   * @param matOutput 输出图像
   * @return 0 成功，负数失败
   */
  static int CenterCropImage(const cv::Mat &matInput, int width, int height,
                              cv::Mat &matOutput);

  // ==================== 翻转 ====================

  /**
   * @brief 翻转图像（cv::Mat 输入版本）
   *
   * @param input 输入图像
   * @param mode 翻转模式：FLIP_HORIZONTAL/FLIP_VERTICAL/FLIP_BOTH
   * @param output 输出图像
   * @return 0 成功，负数失败
   */
  static int FlipImage(const cv::Mat &input, FlipMode mode, cv::Mat &output);

  /**
   * @brief 翻转图像（原始数据输入版本）
   */
  static int FlipImage(const std::string &imageData, FlipMode mode,
                       cv::Mat &output);

  // ==================== 图像拼接/合成 ====================

  /**
   * @brief 水平拼接多张图像
   *
   * @param images 输入图像列表
   * @param output 输出拼接后的图像
   * @return 0 成功，负数失败
   */
  static int HConcat(const std::vector<cv::Mat> &images, cv::Mat &output);

  /**
   * @brief 垂直拼接多张图像
   *
   * @param images 输入图像列表
   * @param output 输出拼接后的图像
   * @return 0 成功，负数失败
   */
  static int VConcat(const std::vector<cv::Mat> &images, cv::Mat &output);

  /**
   * @brief 任意位置叠加合成
   *
   * @param dest 目标图像（会被修改）
   * @param src 源图像
   * @param x 叠加位置 x 坐标
   * @param y 叠加位置 y 坐标
   * @param alpha 透明度 (0.0-1.0)
   * @return 0 成功，负数失败
   */
  static int Overlay(cv::Mat &dest, const cv::Mat &src, int x, int y,
                     double alpha = 1.0);

  // ==================== 文字标注 ====================

  /**
   * @brief 在图像上绘制文字标注
   *
   * @param image 目标图像（会被修改）
   * @param text 标注文字
   * @param position 文字起始位置（左下角）
   * @param opts 标注选项（字体、大小、颜色等）
   * @return 0 成功，负数失败
   */
  static int AnnotateImage(cv::Mat &image, const std::string &text,
                           const cv::Point &position,
                           const AnnotateOptions &opts);

  /**
   * @brief 在原始图像数据上绘制文字标注
   */
  static int AnnotateImage(const std::string &imageData,
                           const std::string &text, const cv::Point &position,
                           const AnnotateOptions &opts, cv::Mat &matOutput);

  // ==================== 写入/导出 ====================

  static int WriteImage(const cv::Mat &mat, const std::string &path);

  /**
   * @brief 写入图像到文件（支持编码参数）
   */
  static int WriteImage(const cv::Mat &mat, const std::string &path,
                        const std::vector<int> &params);

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
  static int AdaptiveWatermarkFill(cv::Mat &dest, const cv::Mat &logo,
                                   cv::Rect region,
                                   int interpolation = cv::INTER_LINEAR,
                                   double alpha = 1.0);

 private:
  static cv::Rect CalculateValidRegion(const cv::Size &image_size,
                                       cv::Rect region);
  static cv::Size CalculateAspectRatioSize(const cv::Size &src_size,
                                           const cv::Size &dst_size);
  static void AlphaBlend(const cv::Mat &background, const cv::Mat &foreground,
                         cv::Mat &dst, double alpha);
};

}  // namespace kcv
}  // namespace kingfisher

#endif
