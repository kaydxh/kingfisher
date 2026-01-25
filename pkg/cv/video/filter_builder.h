#ifndef KINGFISHER_PKG_CV_VIDEO_FILTER_BUILDER_H_
#define KINGFISHER_PKG_CV_VIDEO_FILTER_BUILDER_H_

#include <memory>
#include <string>
#include <vector>

extern "C" {
#include "libavutil/rational.h"
};

namespace kingfisher {
namespace cv {

// ======================= Filter 配置结构体 =======================

// 位置/锚点定义
enum class Position {
  kTopLeft,
  kTopCenter,
  kTopRight,
  kCenterLeft,
  kCenter,
  kCenterRight,
  kBottomLeft,
  kBottomCenter,
  kBottomRight,
  kCustom  // 使用自定义 x, y 坐标
};

// 水印配置
struct WatermarkConfig {
  std::string image_path;  // 水印图片路径
  Position position = Position::kBottomRight;
  int x = 10;              // 自定义 x 坐标（当 position == kCustom 时有效）
  int y = 10;              // 自定义 y 坐标
  int margin = 10;         // 距离边缘的边距
  float opacity = 1.0f;    // 不透明度 [0.0, 1.0]
  float scale = 1.0f;      // 缩放比例
  
  // 动态水印（可选）
  bool enable_fade = false;     // 启用淡入淡出
  double fade_in_start = 0.0;   // 淡入开始时间（秒）
  double fade_in_duration = 1.0; // 淡入持续时间
  double fade_out_start = -1.0; // 淡出开始时间（-1 表示不淡出）
  double fade_out_duration = 1.0;
};

// 文字配置
struct TextConfig {
  std::string text;              // 文字内容
  std::string font_file;         // 字体文件路径（TTF）
  int font_size = 24;            // 字体大小
  std::string font_color = "white";  // 字体颜色（支持 FFmpeg 颜色名称或 0xRRGGBB）
  std::string border_color = "black"; // 描边颜色
  int border_width = 2;          // 描边宽度
  Position position = Position::kBottomLeft;
  int x = 10;
  int y = 10;
  int margin = 10;
  float opacity = 1.0f;
  
  // 阴影效果
  bool enable_shadow = false;
  int shadow_x = 2;
  int shadow_y = 2;
  std::string shadow_color = "black@0.5";
  
  // 背景框
  bool enable_box = false;
  std::string box_color = "black@0.5";
  int box_border_width = 5;
  
  // 时间戳显示（可选）
  bool show_timestamp = false;  // 显示视频时间戳而非固定文字
  std::string timestamp_format = "%H\\:%M\\:%S"; // 时间戳格式
};

// 裁剪配置
struct CropConfig {
  int x = 0;       // 裁剪起始 x 坐标
  int y = 0;       // 裁剪起始 y 坐标
  int width = 0;   // 裁剪宽度（0 表示自动）
  int height = 0;  // 裁剪高度（0 表示自动）
  
  // 居中裁剪（简化配置）
  bool center_crop = false;  // 启用居中裁剪
  int out_width = 0;         // 输出宽度
  int out_height = 0;        // 输出高度
  
  // 保持宽高比裁剪
  bool keep_aspect = false;
  float target_aspect = 0.0f;  // 目标宽高比（例如 16.0/9.0）
};

// 填充/边框配置
struct PadConfig {
  int width = 0;   // 输出宽度（0 表示自动计算）
  int height = 0;  // 输出高度
  int x = -1;      // 视频在画布中的 x 位置（-1 表示居中）
  int y = -1;      // 视频在画布中的 y 位置
  std::string color = "black";  // 填充颜色
  
  // 简化配置：添加边框
  bool add_border = false;
  int border_top = 0;
  int border_bottom = 0;
  int border_left = 0;
  int border_right = 0;
  
  // 等比例填充到指定尺寸
  bool fit_to_size = false;
  int target_width = 0;
  int target_height = 0;
};

// 缩放配置
struct ScaleConfig {
  int width = -1;   // 目标宽度（-1 表示保持比例，-2 表示保持比例且为偶数）
  int height = -1;  // 目标高度
  std::string flags = "bicubic";  // 缩放算法：bicubic, bilinear, lanczos, etc.
  bool force_original_aspect_ratio = false;  // 强制保持原始宽高比
  std::string force_divisible_by = "";  // 强制宽高可被某数整除，如 "2"
};

// 视频拼接模式
enum class ConcatMode {
  kHorizontal,  // 水平拼接（左右）
  kVertical,    // 垂直拼接（上下）
  kGrid,        // 网格布局
  kOverlay,     // 画中画叠加
  kSequential   // 顺序拼接（一个接一个）
};

// 视频拼接配置
struct ConcatConfig {
  ConcatMode mode = ConcatMode::kHorizontal;
  std::vector<std::string> input_files;  // 输入视频文件列表
  
  // 网格布局参数
  int grid_cols = 2;  // 网格列数
  int grid_rows = 2;  // 网格行数
  
  // 画中画参数
  Position pip_position = Position::kBottomRight;
  int pip_x = 10;
  int pip_y = 10;
  float pip_scale = 0.25f;  // 小画面缩放比例
  
  // 输出尺寸（0 表示自动）
  int output_width = 0;
  int output_height = 0;
  
  // 是否同步音频
  bool sync_audio = true;
};

// 颜色调整配置
struct ColorConfig {
  float brightness = 0.0f;   // 亮度调整 [-1.0, 1.0]
  float contrast = 1.0f;     // 对比度 [0.0, 2.0]
  float saturation = 1.0f;   // 饱和度 [0.0, 3.0]
  float gamma = 1.0f;        // Gamma [0.1, 10.0]
  float gamma_r = 1.0f;      // 红色 Gamma
  float gamma_g = 1.0f;      // 绿色 Gamma
  float gamma_b = 1.0f;      // 蓝色 Gamma
};

// 旋转/翻转配置
struct TransformConfig {
  double rotation_angle = 0.0;  // 旋转角度（度）
  bool hflip = false;           // 水平翻转
  bool vflip = false;           // 垂直翻转
  bool transpose = false;       // 转置（90度旋转）
  int transpose_dir = 0;        // 转置方向：0=90°顺时针, 1=90°逆时针, 2=90°顺时针+垂直翻转, 3=90°逆时针+垂直翻转
};

// 模糊配置
struct BlurConfig {
  enum class Type { kBox, kGaussian, kMotion, kSmart };
  Type type = Type::kGaussian;
  int radius = 5;           // 模糊半径
  float sigma = 1.0f;       // 高斯模糊的 sigma
  
  // 区域模糊（可选）
  bool region_blur = false;
  int region_x = 0;
  int region_y = 0;
  int region_width = 0;
  int region_height = 0;
};

// ======================= FilterBuilder 类 =======================

class FilterBuilder {
 public:
  FilterBuilder();
  ~FilterBuilder();

  // 重置所有配置
  FilterBuilder& reset();

  // 水印叠加
  FilterBuilder& add_watermark(const WatermarkConfig& config);
  
  // 文字叠加
  FilterBuilder& add_text(const TextConfig& config);
  
  // 裁剪
  FilterBuilder& crop(const CropConfig& config);
  
  // 填充/边框
  FilterBuilder& pad(const PadConfig& config);
  
  // 缩放
  FilterBuilder& scale(const ScaleConfig& config);
  
  // 缩放（简化版）
  FilterBuilder& scale(int width, int height, const std::string& flags = "bicubic");
  
  // 颜色调整
  FilterBuilder& adjust_color(const ColorConfig& config);
  
  // 旋转/翻转
  FilterBuilder& transform(const TransformConfig& config);
  
  // 模糊
  FilterBuilder& blur(const BlurConfig& config);
  
  // 添加自定义 filter（原始 FFmpeg filter 字符串）
  FilterBuilder& add_custom(const std::string& filter_str);

  // 构建最终的 filter 描述字符串
  std::string build() const;
  
  // 静态辅助方法：生成单个 filter 字符串
  
  // 水印 filter
  static std::string build_watermark_filter(const WatermarkConfig& config,
                                            int video_width, int video_height);
  
  // 文字 filter
  static std::string build_text_filter(const TextConfig& config,
                                       int video_width, int video_height);
  
  // 裁剪 filter
  static std::string build_crop_filter(const CropConfig& config,
                                       int video_width, int video_height);
  
  // 填充 filter
  static std::string build_pad_filter(const PadConfig& config,
                                      int video_width, int video_height);
  
  // 缩放 filter
  static std::string build_scale_filter(const ScaleConfig& config);
  
  // 颜色调整 filter
  static std::string build_color_filter(const ColorConfig& config);
  
  // 旋转/翻转 filter
  static std::string build_transform_filter(const TransformConfig& config);
  
  // 模糊 filter
  static std::string build_blur_filter(const BlurConfig& config,
                                       int video_width, int video_height);
  
  // 视频拼接 filter（复杂 filter graph）
  static std::string build_concat_filter(const ConcatConfig& config);
  
  // 画中画 filter
  static std::string build_pip_filter(const std::string& main_input,
                                      const std::string& overlay_input,
                                      const ConcatConfig& config,
                                      int main_width, int main_height);

 private:
  // 位置计算辅助函数
  static std::pair<std::string, std::string> calculate_position(
      Position pos, int item_width, int item_height,
      int canvas_width, int canvas_height, int margin,
      int custom_x = 0, int custom_y = 0);

  std::vector<std::string> filters_;
};

}  // namespace cv
}  // namespace kingfisher

#endif
