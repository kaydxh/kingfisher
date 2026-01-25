#include "filter_builder.h"

#include <unistd.h>

#include <cmath>
#include <sstream>
#include <vector>

namespace kingfisher {
namespace cv {

// 查找系统可用字体文件
static std::string find_system_font() {
  const std::vector<std::string> font_paths = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    "/usr/share/fonts/gnu-free/FreeSans.ttf",
    "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/google-noto/NotoSans-Regular.ttf",
    "/usr/share/fonts/noto/NotoSans-Regular.ttf",
    "/System/Library/Fonts/Helvetica.ttc",  // macOS
    "/Library/Fonts/Arial.ttf",             // macOS
  };
  for (const auto& path : font_paths) {
    if (access(path.c_str(), R_OK) == 0) {
      return path;
    }
  }
  return "";
}

FilterBuilder::FilterBuilder() {}

FilterBuilder::~FilterBuilder() {}

FilterBuilder& FilterBuilder::reset() {
  filters_.clear();
  return *this;
}

FilterBuilder& FilterBuilder::add_watermark(const WatermarkConfig& config) {
  // 水印需要在最终构建时结合视频尺寸，这里先存储配置
  // 使用占位符，实际应用时需要替换
  std::ostringstream oss;
  
  // 构建 movie 源和 overlay filter
  oss << "movie=" << config.image_path;
  
  // 如果需要缩放水印
  if (config.scale != 1.0f) {
    oss << ",scale=iw*" << config.scale << ":ih*" << config.scale;
  }
  
  // 如果需要调整透明度
  if (config.opacity < 1.0f) {
    oss << ",format=rgba,colorchannelmixer=aa=" << config.opacity;
  }
  
  oss << "[watermark];[in][watermark]overlay=";
  
  // 根据位置设置坐标
  switch (config.position) {
    case Position::kTopLeft:
      oss << config.margin << ":" << config.margin;
      break;
    case Position::kTopCenter:
      oss << "(W-w)/2:" << config.margin;
      break;
    case Position::kTopRight:
      oss << "W-w-" << config.margin << ":" << config.margin;
      break;
    case Position::kCenterLeft:
      oss << config.margin << ":(H-h)/2";
      break;
    case Position::kCenter:
      oss << "(W-w)/2:(H-h)/2";
      break;
    case Position::kCenterRight:
      oss << "W-w-" << config.margin << ":(H-h)/2";
      break;
    case Position::kBottomLeft:
      oss << config.margin << ":H-h-" << config.margin;
      break;
    case Position::kBottomCenter:
      oss << "(W-w)/2:H-h-" << config.margin;
      break;
    case Position::kBottomRight:
      oss << "W-w-" << config.margin << ":H-h-" << config.margin;
      break;
    case Position::kCustom:
      oss << config.x << ":" << config.y;
      break;
  }
  
  // 淡入淡出效果
  if (config.enable_fade) {
    oss << ":enable='between(t," << config.fade_in_start << ","
        << (config.fade_out_start > 0 ? config.fade_out_start + config.fade_out_duration : 9999)
        << ")'";
  }
  
  oss << "[out]";
  
  filters_.push_back(oss.str());
  return *this;
}

FilterBuilder& FilterBuilder::add_text(const TextConfig& config) {
  std::ostringstream oss;
  oss << "drawtext=";
  
  // 字体文件：优先使用用户指定，否则查找系统字体
  std::string font_file = config.font_file;
  if (font_file.empty()) {
    font_file = find_system_font();
  }
  if (!font_file.empty()) {
    oss << "fontfile='" << font_file << "':";
  }
  
  // 文字内容或时间戳
  if (config.show_timestamp) {
    oss << "text='%{pts\\:hms}':";
  } else {
    // 转义特殊字符: ' : backslash
    std::string result;
    for (char c : config.text) {
      if (c == '\'' || c == ':' || c == '\\') {
        result += '\\';
      }
      result += c;
    }
    oss << "text='" << result << "':";
  }
  
  // 字体大小和颜色
  oss << "fontsize=" << config.font_size << ":";
  oss << "fontcolor=" << config.font_color << ":";
  
  // 描边
  if (config.border_width > 0) {
    oss << "borderw=" << config.border_width << ":";
    oss << "bordercolor=" << config.border_color << ":";
  }
  
  // 阴影
  if (config.enable_shadow) {
    oss << "shadowx=" << config.shadow_x << ":";
    oss << "shadowy=" << config.shadow_y << ":";
    oss << "shadowcolor=" << config.shadow_color << ":";
  }
  
  // 背景框
  if (config.enable_box) {
    oss << "box=1:";
    oss << "boxcolor=" << config.box_color << ":";
    oss << "boxborderw=" << config.box_border_width << ":";
  }
  
  // 位置
  switch (config.position) {
    case Position::kTopLeft:
      oss << "x=" << config.margin << ":y=" << config.margin;
      break;
    case Position::kTopCenter:
      oss << "x=(w-text_w)/2:y=" << config.margin;
      break;
    case Position::kTopRight:
      oss << "x=w-text_w-" << config.margin << ":y=" << config.margin;
      break;
    case Position::kCenterLeft:
      oss << "x=" << config.margin << ":y=(h-text_h)/2";
      break;
    case Position::kCenter:
      oss << "x=(w-text_w)/2:y=(h-text_h)/2";
      break;
    case Position::kCenterRight:
      oss << "x=w-text_w-" << config.margin << ":y=(h-text_h)/2";
      break;
    case Position::kBottomLeft:
      oss << "x=" << config.margin << ":y=h-text_h-" << config.margin;
      break;
    case Position::kBottomCenter:
      oss << "x=(w-text_w)/2:y=h-text_h-" << config.margin;
      break;
    case Position::kBottomRight:
      oss << "x=w-text_w-" << config.margin << ":y=h-text_h-" << config.margin;
      break;
    case Position::kCustom:
      oss << "x=" << config.x << ":y=" << config.y;
      break;
  }
  
  filters_.push_back(oss.str());
  return *this;
}

FilterBuilder& FilterBuilder::crop(const CropConfig& config) {
  std::ostringstream oss;
  oss << "crop=";
  
  if (config.center_crop && config.out_width > 0 && config.out_height > 0) {
    // 居中裁剪
    oss << config.out_width << ":" << config.out_height
        << ":(iw-" << config.out_width << ")/2:(ih-" << config.out_height << ")/2";
  } else if (config.keep_aspect && config.target_aspect > 0) {
    // 保持宽高比裁剪
    // 计算裁剪区域以匹配目标宽高比
    oss << "if(gt(a," << config.target_aspect << "),ih*" << config.target_aspect << ",iw)"
        << ":if(gt(a," << config.target_aspect << "),ih,iw/" << config.target_aspect << ")";
  } else {
    // 自定义裁剪
    if (config.width > 0) {
      oss << config.width;
    } else {
      oss << "iw";
    }
    oss << ":";
    if (config.height > 0) {
      oss << config.height;
    } else {
      oss << "ih";
    }
    oss << ":" << config.x << ":" << config.y;
  }
  
  filters_.push_back(oss.str());
  return *this;
}

FilterBuilder& FilterBuilder::pad(const PadConfig& config) {
  std::ostringstream oss;
  
  if (config.add_border) {
    // 添加边框模式
    int new_width = config.border_left + config.border_right;
    int new_height = config.border_top + config.border_bottom;
    oss << "pad=iw+" << new_width << ":ih+" << new_height
        << ":" << config.border_left << ":" << config.border_top
        << ":" << config.color;
  } else if (config.fit_to_size && config.target_width > 0 && config.target_height > 0) {
    // 等比例填充到指定尺寸
    // 先缩放保持比例，再填充到目标尺寸
    oss << "scale=" << config.target_width << ":" << config.target_height
        << ":force_original_aspect_ratio=decrease,";
    oss << "pad=" << config.target_width << ":" << config.target_height
        << ":(ow-iw)/2:(oh-ih)/2:" << config.color;
  } else {
    // 自定义填充
    oss << "pad=";
    if (config.width > 0) {
      oss << config.width;
    } else {
      oss << "iw";
    }
    oss << ":";
    if (config.height > 0) {
      oss << config.height;
    } else {
      oss << "ih";
    }
    oss << ":";
    if (config.x >= 0) {
      oss << config.x;
    } else {
      oss << "(ow-iw)/2";  // 居中
    }
    oss << ":";
    if (config.y >= 0) {
      oss << config.y;
    } else {
      oss << "(oh-ih)/2";  // 居中
    }
    oss << ":" << config.color;
  }
  
  filters_.push_back(oss.str());
  return *this;
}

FilterBuilder& FilterBuilder::scale(const ScaleConfig& config) {
  std::ostringstream oss;
  oss << "scale=";
  
  if (config.width > 0) {
    oss << config.width;
  } else {
    oss << config.width;  // -1 或 -2
  }
  oss << ":";
  if (config.height > 0) {
    oss << config.height;
  } else {
    oss << config.height;
  }
  
  // 缩放算法
  oss << ":flags=" << config.flags;
  
  // 保持原始宽高比
  if (config.force_original_aspect_ratio) {
    oss << ":force_original_aspect_ratio=decrease";
  }
  
  // 强制宽高可被某数整除
  if (!config.force_divisible_by.empty()) {
    oss << ":force_divisible_by=" << config.force_divisible_by;
  }
  
  filters_.push_back(oss.str());
  return *this;
}

FilterBuilder& FilterBuilder::scale(int width, int height, const std::string& flags) {
  ScaleConfig config;
  config.width = width;
  config.height = height;
  config.flags = flags;
  return scale(config);
}

FilterBuilder& FilterBuilder::adjust_color(const ColorConfig& config) {
  std::ostringstream oss;
  oss << "eq=";
  oss << "brightness=" << config.brightness << ":";
  oss << "contrast=" << config.contrast << ":";
  oss << "saturation=" << config.saturation << ":";
  oss << "gamma=" << config.gamma << ":";
  oss << "gamma_r=" << config.gamma_r << ":";
  oss << "gamma_g=" << config.gamma_g << ":";
  oss << "gamma_b=" << config.gamma_b;
  
  filters_.push_back(oss.str());
  return *this;
}

FilterBuilder& FilterBuilder::transform(const TransformConfig& config) {
  // 水平翻转
  if (config.hflip) {
    filters_.push_back("hflip");
  }
  
  // 垂直翻转
  if (config.vflip) {
    filters_.push_back("vflip");
  }
  
  // 转置（90度旋转）
  if (config.transpose) {
    filters_.push_back("transpose=" + std::to_string(config.transpose_dir));
  }
  
  // 任意角度旋转
  if (std::abs(config.rotation_angle) > 0.001) {
    std::ostringstream oss;
    double radians = config.rotation_angle * M_PI / 180.0;
    oss << "rotate=" << radians << ":ow=rotw(" << radians << "):oh=roth(" << radians << ")";
    filters_.push_back(oss.str());
  }
  
  return *this;
}

FilterBuilder& FilterBuilder::blur(const BlurConfig& config) {
  std::ostringstream oss;
  
  std::string blur_filter;
  switch (config.type) {
    case BlurConfig::Type::kBox:
      blur_filter = "boxblur=" + std::to_string(config.radius) + ":" +
                    std::to_string(config.radius);
      break;
    case BlurConfig::Type::kGaussian:
      blur_filter = "gblur=sigma=" + std::to_string(config.sigma);
      break;
    case BlurConfig::Type::kMotion:
      blur_filter = "avgblur=sizeX=" + std::to_string(config.radius) +
                    ":sizeY=1";  // 水平运动模糊
      break;
    case BlurConfig::Type::kSmart:
      blur_filter = "smartblur=lr=" + std::to_string(config.radius) +
                    ":ls=" + std::to_string(config.sigma);
      break;
  }
  
  if (config.region_blur && config.region_width > 0 && config.region_height > 0) {
    // 区域模糊：使用 split + crop + overlay 实现
    oss << "split[main][blur];"
        << "[blur]crop=" << config.region_width << ":" << config.region_height
        << ":" << config.region_x << ":" << config.region_y
        << "," << blur_filter << "[blurred];"
        << "[main][blurred]overlay=" << config.region_x << ":" << config.region_y;
    filters_.push_back(oss.str());
  } else {
    filters_.push_back(blur_filter);
  }
  
  return *this;
}

FilterBuilder& FilterBuilder::add_custom(const std::string& filter_str) {
  if (!filter_str.empty()) {
    filters_.push_back(filter_str);
  }
  return *this;
}

std::string FilterBuilder::build() const {
  if (filters_.empty()) {
    return "null";  // 空 filter
  }
  
  std::ostringstream oss;
  for (size_t i = 0; i < filters_.size(); ++i) {
    if (i > 0) {
      oss << ",";
    }
    oss << filters_[i];
  }
  return oss.str();
}

// ======================= 静态方法实现 =======================

std::string FilterBuilder::build_watermark_filter(const WatermarkConfig& config,
                                                   int video_width,
                                                   int video_height) {
  FilterBuilder builder;
  builder.add_watermark(config);
  return builder.build();
}

std::string FilterBuilder::build_text_filter(const TextConfig& config,
                                              int video_width,
                                              int video_height) {
  FilterBuilder builder;
  builder.add_text(config);
  return builder.build();
}

std::string FilterBuilder::build_crop_filter(const CropConfig& config,
                                              int video_width,
                                              int video_height) {
  FilterBuilder builder;
  builder.crop(config);
  return builder.build();
}

std::string FilterBuilder::build_pad_filter(const PadConfig& config,
                                             int video_width,
                                             int video_height) {
  FilterBuilder builder;
  builder.pad(config);
  return builder.build();
}

std::string FilterBuilder::build_scale_filter(const ScaleConfig& config) {
  FilterBuilder builder;
  builder.scale(config);
  return builder.build();
}

std::string FilterBuilder::build_color_filter(const ColorConfig& config) {
  FilterBuilder builder;
  builder.adjust_color(config);
  return builder.build();
}

std::string FilterBuilder::build_transform_filter(const TransformConfig& config) {
  FilterBuilder builder;
  builder.transform(config);
  return builder.build();
}

std::string FilterBuilder::build_blur_filter(const BlurConfig& config,
                                              int video_width,
                                              int video_height) {
  FilterBuilder builder;
  builder.blur(config);
  return builder.build();
}

std::string FilterBuilder::build_concat_filter(const ConcatConfig& config) {
  std::ostringstream oss;
  int n = static_cast<int>(config.input_files.size());
  
  if (n < 2) {
    return "null";
  }
  
  switch (config.mode) {
    case ConcatMode::kHorizontal: {
      // 水平拼接：[0][1]hstack=inputs=2
      for (int i = 0; i < n; ++i) {
        oss << "[" << i << "]";
      }
      oss << "hstack=inputs=" << n;
      break;
    }
    
    case ConcatMode::kVertical: {
      // 垂直拼接：[0][1]vstack=inputs=2
      for (int i = 0; i < n; ++i) {
        oss << "[" << i << "]";
      }
      oss << "vstack=inputs=" << n;
      break;
    }
    
    case ConcatMode::kGrid: {
      // 网格布局：使用 xstack
      int cols = config.grid_cols;
      int rows = config.grid_rows;
      
      // 构建布局字符串
      // layout 格式：0_0|w0_0|w0+w1_0|0_h0|w0_h0|...
      std::string layout;
      for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
          int idx = r * cols + c;
          if (idx >= n) break;
          
          if (!layout.empty()) {
            layout += "|";
          }
          
          // x 坐标
          if (c == 0) {
            layout += "0";
          } else {
            for (int i = 0; i < c; ++i) {
              if (i > 0) layout += "+";
              layout += "w" + std::to_string(r * cols + i);
            }
          }
          layout += "_";
          
          // y 坐标
          if (r == 0) {
            layout += "0";
          } else {
            for (int i = 0; i < r; ++i) {
              if (i > 0) layout += "+";
              layout += "h" + std::to_string(i * cols);
            }
          }
        }
      }
      
      for (int i = 0; i < n; ++i) {
        oss << "[" << i << "]";
      }
      oss << "xstack=inputs=" << n << ":layout=" << layout;
      break;
    }
    
    case ConcatMode::kSequential: {
      // 顺序拼接（时间上）
      for (int i = 0; i < n; ++i) {
        oss << "[" << i << ":v][" << i << ":a]";
      }
      oss << "concat=n=" << n << ":v=1:a=" << (config.sync_audio ? 1 : 0);
      break;
    }
    
    case ConcatMode::kOverlay: {
      // 画中画模式在 build_pip_filter 中实现
      // 这里返回简单的第一个视频
      oss << "null";
      break;
    }
  }
  
  return oss.str();
}

std::string FilterBuilder::build_pip_filter(const std::string& main_input,
                                             const std::string& overlay_input,
                                             const ConcatConfig& config,
                                             int main_width,
                                             int main_height) {
  std::ostringstream oss;
  
  // 缩放小画面
  int pip_width = static_cast<int>(main_width * config.pip_scale);
  int pip_height = static_cast<int>(main_height * config.pip_scale);
  
  oss << "[" << overlay_input << "]scale=" << pip_width << ":" << pip_height << "[pip];";
  oss << "[" << main_input << "][pip]overlay=";
  
  // 位置计算
  int margin = 10;
  switch (config.pip_position) {
    case Position::kTopLeft:
      oss << margin << ":" << margin;
      break;
    case Position::kTopRight:
      oss << "W-w-" << margin << ":" << margin;
      break;
    case Position::kBottomLeft:
      oss << margin << ":H-h-" << margin;
      break;
    case Position::kBottomRight:
    default:
      oss << "W-w-" << margin << ":H-h-" << margin;
      break;
    case Position::kCustom:
      oss << config.pip_x << ":" << config.pip_y;
      break;
    case Position::kCenter:
      oss << "(W-w)/2:(H-h)/2";
      break;
    case Position::kTopCenter:
      oss << "(W-w)/2:" << margin;
      break;
    case Position::kBottomCenter:
      oss << "(W-w)/2:H-h-" << margin;
      break;
    case Position::kCenterLeft:
      oss << margin << ":(H-h)/2";
      break;
    case Position::kCenterRight:
      oss << "W-w-" << margin << ":(H-h)/2";
      break;
  }
  
  return oss.str();
}

std::pair<std::string, std::string> FilterBuilder::calculate_position(
    Position pos, int item_width, int item_height,
    int canvas_width, int canvas_height, int margin,
    int custom_x, int custom_y) {
  std::string x_expr, y_expr;
  
  switch (pos) {
    case Position::kTopLeft:
      x_expr = std::to_string(margin);
      y_expr = std::to_string(margin);
      break;
    case Position::kTopCenter:
      x_expr = "(" + std::to_string(canvas_width) + "-" + std::to_string(item_width) + ")/2";
      y_expr = std::to_string(margin);
      break;
    case Position::kTopRight:
      x_expr = std::to_string(canvas_width - item_width - margin);
      y_expr = std::to_string(margin);
      break;
    case Position::kCenterLeft:
      x_expr = std::to_string(margin);
      y_expr = "(" + std::to_string(canvas_height) + "-" + std::to_string(item_height) + ")/2";
      break;
    case Position::kCenter:
      x_expr = "(" + std::to_string(canvas_width) + "-" + std::to_string(item_width) + ")/2";
      y_expr = "(" + std::to_string(canvas_height) + "-" + std::to_string(item_height) + ")/2";
      break;
    case Position::kCenterRight:
      x_expr = std::to_string(canvas_width - item_width - margin);
      y_expr = "(" + std::to_string(canvas_height) + "-" + std::to_string(item_height) + ")/2";
      break;
    case Position::kBottomLeft:
      x_expr = std::to_string(margin);
      y_expr = std::to_string(canvas_height - item_height - margin);
      break;
    case Position::kBottomCenter:
      x_expr = "(" + std::to_string(canvas_width) + "-" + std::to_string(item_width) + ")/2";
      y_expr = std::to_string(canvas_height - item_height - margin);
      break;
    case Position::kBottomRight:
      x_expr = std::to_string(canvas_width - item_width - margin);
      y_expr = std::to_string(canvas_height - item_height - margin);
      break;
    case Position::kCustom:
      x_expr = std::to_string(custom_x);
      y_expr = std::to_string(custom_y);
      break;
  }
  
  return {x_expr, y_expr};
}

}  // namespace cv
}  // namespace kingfisher
