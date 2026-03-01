
#ifndef KINGFISHER_PKG_CV_IMAGE_PIPELINE_H_
#define KINGFISHER_PKG_CV_IMAGE_PIPELINE_H_

#include <string>
#include <vector>

#include "image.pb.h"

#ifdef ENABLE_OPENCV
#if (CV_MAJOR_VERSION >= 4)
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#else
#include <opencv2/opencv.hpp>
#endif
#endif  // ENABLE_OPENCV

namespace kingfisher {
namespace kcv {

// 前向声明 Image 类（ImagePipeline 内部调用 Image 的 static 方法）
class Image;

// ==================== 面向对象链式调用 API ====================

/**
 * @brief 图像处理管道类，持有 cv::Mat 内部状态，支持链式调用
 *
 * 在保留 Image 类全部 static API 的基础上，提供面向对象的流式处理接口。
 * 每个操作方法返回 *this 引用，可链式串联多个操作。
 * 操作过程中的错误会被记录，可通过 ok()/error_code()/error_message() 查询。
 *
 * 使用示例：
 * @code
 *   ImagePipeline pipeline;
 *   pipeline.decode(imageData, opts)
 *          .resize(800, 600)
 *          .rotate(90)
 *          .gaussianBlur(5, 1.0)
 *          .sharpen(1.5)
 *          .adjustBrightnessContrast(10, 1.2);
 *
 *   if (pipeline.ok()) {
 *       cv::Mat result = pipeline.mat();
 *       std::string encoded = pipeline.encode(".jpg", 95);
 *   }
 * @endcode
 *
 * 错误传播规则：
 * - 一旦某个操作失败，后续操作将自动跳过（短路机制）
 * - 可调用 reset() 清除错误状态并重新开始
 */
class ImagePipeline {
 public:
  // ==================== 构造/初始化 ====================

  /**
   * @brief 默认构造，创建空管道
   */
  ImagePipeline();

  /**
   * @brief 从 cv::Mat 构造
   */
  explicit ImagePipeline(const cv::Mat &mat);

  /**
   * @brief 从图像字节数据构造并解码
   */
  explicit ImagePipeline(const std::string &imageData,
                         const DecodeOptions &opts = DecodeOptions());

  /**
   * @brief 拷贝构造
   */
  ImagePipeline(const ImagePipeline &other);

  /**
   * @brief 移动构造
   */
  ImagePipeline(ImagePipeline &&other) noexcept;

  /**
   * @brief 拷贝赋值
   */
  ImagePipeline& operator=(const ImagePipeline &other);

  /**
   * @brief 移动赋值
   */
  ImagePipeline& operator=(ImagePipeline &&other) noexcept;

  ~ImagePipeline() = default;

  // ==================== 状态查询 ====================

  /**
   * @brief 是否处于正常状态（没有错误）
   */
  bool ok() const;

  /**
   * @brief 内部图像是否为空
   */
  bool empty() const;

  /**
   * @brief 获取最后的错误码
   */
  int error_code() const;

  /**
   * @brief 获取最后的错误消息
   */
  const std::string& error_message() const;

  /**
   * @brief 获取内部 cv::Mat 的常量引用
   */
  const cv::Mat& mat() const;

  /**
   * @brief 获取内部 cv::Mat 的可修改引用
   */
  cv::Mat& mat();

  /**
   * @brief 获取图像宽度
   */
  int width() const;

  /**
   * @brief 获取图像高度
   */
  int height() const;

  /**
   * @brief 获取图像通道数
   */
  int channels() const;

  // ==================== 重置/加载 ====================

  /**
   * @brief 重置管道状态，清除错误和图像数据
   */
  ImagePipeline& reset();

  /**
   * @brief 从字节数据解码图像
   */
  ImagePipeline& decode(const std::string &imageData,
                        const DecodeOptions &opts = DecodeOptions());

  /**
   * @brief 从文件解码图像
   */
  ImagePipeline& decodeFile(const std::string &imageFile,
                            const DecodeOptions &opts = DecodeOptions());

  /**
   * @brief 设置内部 cv::Mat
   */
  ImagePipeline& load(const cv::Mat &mat);

  // ==================== 几何变换（链式） ====================

  /**
   * @brief 旋转图像
   * @param angle 顺时针旋转角度
   */
  ImagePipeline& rotate(double angle);

  /**
   * @brief 缩放图像
   * @param width 目标宽度
   * @param height 目标高度
   * @param keepRatio 是否保持宽高比
   */
  ImagePipeline& resize(int width, int height, bool keepRatio = false);

  /**
   * @brief 裁剪图像
   */
  ImagePipeline& crop(const Rect &rect);

  /**
   * @brief 中心裁剪
   */
  ImagePipeline& centerCrop(int width, int height);

  /**
   * @brief 翻转图像
   */
  ImagePipeline& flip(FlipMode mode);

  /**
   * @brief 生成缩略图
   */
  ImagePipeline& thumbnail(int maxDimension);

  // ==================== 颜色/滤镜（链式） ====================

  /**
   * @brief 颜色空间转换
   */
  ImagePipeline& convertColorSpace(ColorSpace from, ColorSpace to);

  /**
   * @brief 灰度化
   */
  ImagePipeline& grayscale();

  /**
   * @brief 高斯模糊
   */
  ImagePipeline& gaussianBlur(int kernelSize = 5, double sigma = 0);

  /**
   * @brief 均值模糊
   */
  ImagePipeline& meanBlur(int kernelSize = 5);

  /**
   * @brief 中值模糊
   */
  ImagePipeline& medianBlur(int kernelSize = 5);

  /**
   * @brief 双边滤波
   */
  ImagePipeline& bilateralFilter(int d = 9, double sigmaColor = 75,
                                  double sigmaSpace = 75);

  /**
   * @brief 图像锐化
   */
  ImagePipeline& sharpen(double amount = 1.0, int kernelSize = 3);

  /**
   * @brief 调整亮度和对比度
   */
  ImagePipeline& adjustBrightnessContrast(double brightness = 0,
                                           double contrast = 1.0);

  // ==================== 合成/标注（链式） ====================

  /**
   * @brief 叠加合成另一张图像
   */
  ImagePipeline& overlay(const cv::Mat &src, int x, int y,
                          double alpha = 1.0);

  /**
   * @brief 自适应水印填充
   */
  ImagePipeline& watermark(const cv::Mat &logo, cv::Rect region,
                            int interpolation = cv::INTER_LINEAR,
                            double alpha = 1.0);

  /**
   * @brief 文字标注
   */
  ImagePipeline& annotate(const std::string &text, const cv::Point &position,
                           const AnnotateOptions &opts = AnnotateOptions());

  // ==================== 输出 ====================

  /**
   * @brief 编码为指定格式的内存 buffer
   * @param format 编码格式，如 ".jpg", ".png"
   * @param quality 编码质量
   * @return 编码后的字节数据（失败返回空字符串）
   */
  std::string encode(const std::string &format, int quality = 90);

  /**
   * @brief 写入文件
   * @param path 输出文件路径
   * @return 0 成功，负数失败
   */
  int writeTo(const std::string &path);

  /**
   * @brief 写入文件（带参数）
   */
  int writeTo(const std::string &path, const std::vector<int> &params);

  /**
   * @brief 克隆当前管道（深拷贝）
   */
  ImagePipeline clone() const;

 private:
  /**
   * @brief 设置错误状态
   */
  void setError(int code, const std::string &msg);

  cv::Mat mat_;               // 内部图像数据
  int error_code_;            // 错误码，0 表示正常
  std::string error_message_; // 错误消息
};

}  // namespace kcv
}  // namespace kingfisher

#endif
