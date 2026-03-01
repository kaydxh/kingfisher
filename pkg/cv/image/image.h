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

  // ==================== 缩略图 ====================

  /**
   * @brief 生成缩略图（指定最大边长，自动保持宽高比）
   *
   * @param imageData 原始图像字节数据
   * @param maxDimension 最大边长（长边不超过此值）
   * @param matOutput 输出图像
   * @return 0 成功，负数失败
   */
  static int Thumbnail(const std::string &imageData, int maxDimension,
                       cv::Mat &matOutput);

  /**
   * @brief 生成缩略图（cv::Mat 输入版本）
   */
  static int Thumbnail(const cv::Mat &matInput, int maxDimension,
                       cv::Mat &matOutput);

  // ==================== 图像滤镜 ====================

  /**
   * @brief 高斯模糊
   *
   * @param input 输入图像
   * @param output 输出图像
   * @param kernelSize 核大小（必须为正奇数，默认5）
   * @param sigma sigma值（默认0，自动计算）
   * @return 0 成功，负数失败
   */
  static int GaussianBlur(const cv::Mat &input, cv::Mat &output,
                          int kernelSize = 5, double sigma = 0);

  /**
   * @brief 均值模糊
   */
  static int MeanBlur(const cv::Mat &input, cv::Mat &output,
                      int kernelSize = 5);

  /**
   * @brief 中值模糊
   */
  static int MedianBlur(const cv::Mat &input, cv::Mat &output,
                        int kernelSize = 5);

  /**
   * @brief 双边滤波（保边平滑）
   */
  static int BilateralFilter(const cv::Mat &input, cv::Mat &output,
                             int d = 9, double sigmaColor = 75,
                             double sigmaSpace = 75);

  /**
   * @brief 图像模糊（统一接口，通过 FilterOptions 指定模糊类型和参数）
   */
  static int BlurImage(const cv::Mat &input, const FilterOptions &opts,
                       cv::Mat &output);

  /**
   * @brief 图像锐化（USM 锐化）
   *
   * @param input 输入图像
   * @param output 输出图像
   * @param amount 锐化强度（默认1.0）
   * @param kernelSize 高斯核大小（默认3）
   * @return 0 成功，负数失败
   */
  static int Sharpen(const cv::Mat &input, cv::Mat &output,
                     double amount = 1.0, int kernelSize = 3);

  /**
   * @brief 调整亮度和对比度
   *
   * @param input 输入图像
   * @param output 输出图像
   * @param brightness 亮度偏移 [-255, 255]（默认0）
   * @param contrast 对比度系数 [0.0, 3.0]（默认1.0）
   * @return 0 成功，负数失败
   */
  static int AdjustBrightnessContrast(const cv::Mat &input, cv::Mat &output,
                                      double brightness = 0,
                                      double contrast = 1.0);

  /**
   * @brief 灰度化
   */
  static int Grayscale(const cv::Mat &input, cv::Mat &output);

  // ==================== 图像相似度 ====================

  /**
   * @brief 计算图像哈希（aHash/pHash/dHash）
   *
   * @param input 输入图像
   * @param hashType 哈希类型
   * @param hashValue 输出哈希值（64位）
   * @return 0 成功，负数失败
   */
  static int ComputeImageHash(const cv::Mat &input, HashType hashType,
                              uint64_t &hashValue);

  /**
   * @brief 计算两个图像哈希的汉明距离
   *
   * @param hash1 第一个哈希值
   * @param hash2 第二个哈希值
   * @return 汉明距离 (0-64)，值越小越相似
   */
  static int HammingDistance(uint64_t hash1, uint64_t hash2);

  /**
   * @brief 计算 SSIM（结构相似度）
   *
   * @param img1 第一张图像
   * @param img2 第二张图像（尺寸必须与 img1 相同）
   * @return SSIM 值 [0, 1]，1 表示完全相同；负数表示错误
   */
  static double ComputeSSIM(const cv::Mat &img1, const cv::Mat &img2);

  /**
   * @brief 直方图比较
   *
   * @param img1 第一张图像
   * @param img2 第二张图像
   * @param method 比较方法
   * @return 相似度得分；负数表示错误
   */
  static double CompareHistogram(const cv::Mat &img1, const cv::Mat &img2,
                                 CompareMethod method = COMPARE_CORRELATION);

  // ==================== EXIF 信息 ====================

  /**
   * @brief 读取完整的 EXIF 信息
   *
   * @param imageData 原始图像字节数据
   * @param exifInfo 输出的 EXIF 信息
   * @return 0 成功，负数失败
   */
  static int ReadExifInfo(const std::string &imageData, ExifInfo &exifInfo);

  // ==================== 批量处理 ====================

  /**
   * @brief 批量解码图像
   *
   * @param imageDatas 输入的图像数据列表
   * @param opts 解码选项
   * @param matOutputs 输出的图像列表
   * @return 成功解码的数量（负数表示全部失败）
   */
  static int BatchDecodeImages(
      const std::vector<std::string> &imageDatas, const DecodeOptions &opts,
      std::vector<cv::Mat> &matOutputs);

  /**
   * @brief 批量缩放图像
   */
  static int BatchResizeImages(
      const std::vector<cv::Mat> &inputs, int width, int height,
      bool keepRatio, std::vector<cv::Mat> &outputs);

  // ==================== Mat ↔ Magick::Image 转换 ====================

  /**
   * @brief cv::Mat 转换为 Magick::Image（反向转换）
   *
   * @param mat 输入的 cv::Mat（支持 BGR/BGRA/GRAY）
   * @param imageOutput 输出的 Magick::Image
   * @return 0 成功，负数失败
   */
  static int MatToImage(const cv::Mat &mat, Magick::Image &imageOutput);

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
