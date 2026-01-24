# Kingfisher 视频处理模块问题解决总结

本文档总结了 Kingfisher 项目中视频处理模块（`pkg/cv/video`）在开发过程中遇到的所有问题及其解决方案。

---

## 目录

1. [编译问题](#1-编译问题)
2. [Core Dump 问题](#2-core-dump-问题)
3. [moov atom not found 问题](#3-moov-atom-not-found-问题)
4. [PTS（时间戳）问题](#4-pts时间戳问题)
5. [FPS（帧率）问题](#5-fps帧率问题)
6. [特殊视频流处理问题](#6-特殊视频流处理问题)
7. [自定义过滤器支持](#7-自定义过滤器支持)
8. [测试程序使用说明](#8-测试程序使用说明)
9. [FFmpeg 核心概念](#9-ffmpeg-核心概念)
10. [音视频同步机制](#10-音视频同步机制)
11. [CUDA 硬件编解码支持](#11-cuda-硬件编解码支持)

---

## 1. 编译问题

### 1.1 FFmpeg 头文件包含错误

**问题描述**：
编译时出现 FFmpeg 头文件找不到或符号未定义的错误。

**原因分析**：
- FFmpeg 的 C 头文件需要用 `extern "C"` 包裹才能在 C++ 中正确链接
- 某些 FFmpeg 类型（如 `AVRational`、`AVCodecID` 等）需要包含特定头文件

**解决方案**：
```cpp
// 正确的包含方式
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/frame.h"
#include "libavutil/rational.h"
}
```

### 1.2 shared_ptr 与 FFmpeg 对象的内存管理

**问题描述**：
使用 `std::shared_ptr` 管理 FFmpeg 对象时出现编译错误或运行时 double-free。

**原因分析**：
FFmpeg 对象有特定的分配和释放函数，不能直接使用 `delete`。

**解决方案**：
使用自定义删除器：
```cpp
// AVFrame
std::shared_ptr<AVFrame> frame(
    av_frame_alloc(), 
    [](AVFrame *f) { av_frame_free(&f); }
);

// AVPacket
std::shared_ptr<AVPacket> pkt(
    av_packet_alloc(), 
    [](AVPacket *p) { av_packet_free(&p); }
);

// AVFormatContext（输入）
std::shared_ptr<AVFormatContext> ifmt_ctx(
    avformat_alloc_context(),
    [](AVFormatContext *ctx) { avformat_close_input(&ctx); }
);

// AVFormatContext（输出）
std::shared_ptr<AVFormatContext> ofmt_ctx(
    ofmt_ctx,
    [](AVFormatContext *ctx) {
        if (ctx && !(ctx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&ctx->pb);
        }
        avformat_free_context(ctx);
    }
);
```

**相关文件**：
- `pkg/cv/video/stream.cc`
- `pkg/cv/video/input_file.cc`
- `pkg/cv/video/output_file.cc`

### 1.3 CMakeLists.txt 编译配置问题

**问题描述**：
项目编译时出现以下问题：
1. protobuf 版本冲突（grpc 内置 3.12.2 vs 项目 3.6.1）
2. gtest 找不到或链接失败
3. C++ ABI 兼容性问题
4. GraphicsMagick 依赖路径变更

**解决方案（Commit dba19ff）**：

#### 1.3.1 添加 C++11 ABI 兼容性标志

```cmake
# CMakeLists.txt
set(CXX_FLAGS
 -march=native
 -std=c++17
 -rdynamic
 -D_GLIBCXX_USE_CXX11_ABI=0  # 新增：解决 ABI 兼容性问题
)
```

#### 1.3.2 修改 gtest 集成方式

```cmake
# CMakeLists.txt - 修改前（仅检查头文件）
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/include/gtest/gtest.h OR
    EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/include/gtest/gtest.h)
  add_subdirectory (test)
endif()

# CMakeLists.txt - 修改后（正确添加 gtest 子项目）
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/CMakeLists.txt)
  add_subdirectory(third_party/gtest)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/googletest/include)
  add_subdirectory(test)
elseif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/CMakeLists.txt)
  add_subdirectory(third_party/googletest)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/googletest/include)
  add_subdirectory(test)
else()
  message(STATUS "Skipping test directory (gtest not found)")
endif()
```

#### 1.3.3 统一 protobuf 版本，避免与 grpc 冲突

```cmake
# cmake/Build.options.cmake - protobuf 配置
if (ENABLE_PROTOBUF)
  message(STATUS "> build with protobuf lib")
  add_definitions(-DENABLE_PROTOBUF)
  # 强制使用 protobuf 3.6.1（统一版本，不使用 grpc 的 protobuf）
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/protobuf-v3.6.1/include)
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/protobuf-v3.6.1/lib)
  message(STATUS "> using protobuf 3.6.1 (forced)")
endif()
```

#### 1.3.4 默认关闭 BRPC 和 GRPC 以避免版本冲突

```cmake
# cmake/Build.options.cmake
option(ENABLE_BRPC "ENABLE_BRPC" OFF)  # 修改: ON -> OFF
option(ENABLE_GRPC "ENABLE_GRPC" OFF)  # 修改: ON -> OFF
```

#### 1.3.5 更新 GraphicsMagick 依赖路径并添加 jbig 库

```cmake
# cmake/Build.options.cmake - GraphicsMagick 配置
if (ENABLE_GRAPHICS_MAGICK)
  message(STATUS " > build with graphics magick lib")
  add_definitions(-DENABLE_GRAPHICS_MAGICK)
  # 修改：更新依赖列表，添加 jbig 库
  set(MAGICK_DEPS z bz2 gomp GraphicsMagickWand GraphicsMagick++ GraphicsMagick tiff jbig)
  # 修改：更新路径
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/graphics-magick/include)
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/graphics-magick/lib)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/jbig/include)
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/jbig/lib64)
endif()
```

#### 1.3.6 完善测试程序链接依赖

```cmake
# test/CMakeLists.txt
target_link_libraries (${TARGET_NAME}
  kingfisher_base
  kingfisher_pkg
  proto-image
  gtest
  ${OPENCV_DEPS}
  ${MAGICK_DEPS}
  ${PROTOBUF_DEPS}
  # ${GRPC_DEPS} - 暂时移除，避免 protobuf 版本冲突
  pthread
  dl
  z
  rt)
```

**相关文件**：
- `CMakeLists.txt`
- `cmake/Build.options.cmake`
- `test/CMakeLists.txt`

---

## 2. Core Dump 问题

### 2.1 空指针访问 - Filter Context

**问题描述**：
在初始化输出流编码器时，访问 `filter_->filter_` 导致 segmentation fault。

**原因分析**：
当不使用 filter 时，`ost->filter_` 为 `nullptr`，直接访问其成员会崩溃。

**解决方案**：
在访问 filter context 前进行空指针检查：
```cpp
// output_file.cc: init_output_stream_encode()
// 安全获取 filter context（可能为 null）
AVFilterContext *filter_ctx = 
    (ost->filter_ && ost->filter_->filter_) ? ost->filter_->filter_ : nullptr;
```

### 2.2 AVFormatContext 释放顺序问题

**问题描述**：
在 `OutputFile` 析构时崩溃。

**原因分析**：
在写入 trailer 之前释放了相关资源，或者在 IO context 关闭之前释放了 format context。

**解决方案**：
在析构函数中正确的资源释放顺序：
```cpp
OutputFile::~OutputFile() {
  // 在析构时自动 flush，确保 moov atom 被正确写入
  flush();
  
  // 然后释放字典
  av_dict_free(&command_opts_);
  av_dict_free(&encoder_opts_);
  av_dict_free(&opts_);
}
```

### 2.3 未初始化的编码器

**问题描述**：
在编码帧之前检查编码器是否已打开。

**原因分析**：
如果在 `avcodec_open2` 之前调用 `avcodec_send_frame`，会导致未定义行为。

**解决方案**：
```cpp
// output_file.cc: of_encode_frame()
if (!avcodec_is_open(enc_ctx.get())) {
  if (frame) {
    av_log(this, AV_LOG_ERROR,
           "encoder is not open before encode for output stream #%d:%d\n",
           file_index_, ost->stream_index_);
    return AVERROR(EINVAL);
  }
}
```

### 2.4 avformat_open_input 指针修改问题

**问题描述**：
运行特殊视频时程序 core dump，堆栈显示崩溃在 `avformat_find_stream_info` -> `av_dict_get`。

**原因分析**：
在构造函数中提前创建了 `shared_ptr<AVFormatContext>`，但 `avformat_open_input` 函数会修改传入的指针：

```cpp
// 错误代码
InputFile::InputFile()
    : ifmt_ctx_(std::shared_ptr<AVFormatContext>(
          avformat_alloc_context(),  // ❌ 过早创建 shared_ptr
          [](AVFormatContext *ctx) { avformat_close_input(&ctx); })),
      ...

int InputFile::open(...) {
  AVFormatContext *ifmt_ctx = ifmt_ctx_.get();  // 获取指针副本
  ret = avformat_open_input(&ifmt_ctx, ...);    // ❌ 可能修改 ifmt_ctx 指针！
  // 此时 ifmt_ctx_ 仍指向旧地址（可能已无效）
}
```

**解决方案**：
参照 ffmpeg-wrapper 的实现，在 `avformat_open_input` 成功后才创建 `shared_ptr`：

```cpp
// 修复后的代码
InputFile::InputFile() : pkt_(av_packet_alloc()), av_class_(...) {
  // 不在构造函数创建 ifmt_ctx_
}

int InputFile::open(...) {
  AVFormatContext *ifmt_ctx = avformat_alloc_context();
  ifmt_ctx->flags |= AVFMT_FLAG_NONBLOCK;
  
  ret = avformat_open_input(&ifmt_ctx, ...);
  if (ret < 0) return ret;
  
  // ✅ avformat_open_input 成功后才创建 shared_ptr
  ifmt_ctx_ = std::shared_ptr<AVFormatContext>(
      ifmt_ctx, [](AVFormatContext *ctx) { avformat_close_input(&ctx); });
}
```

### 2.5 avformat_find_stream_info 选项问题

**问题描述**：
某些特殊视频在调用 `avformat_find_stream_info` 时崩溃。

**原因分析**：
`avformat_find_stream_info` 的第二个参数需要为每个流单独创建解码选项，而不是直接传递单个 `AVDictionary*`。

**解决方案**：
添加 `setup_find_stream_info_opts` 函数：

```cpp
// ffmpeg_utils.cc
AVDictionary **setup_find_stream_info_opts(AVFormatContext *s,
                                           AVDictionary *codec_opts) {
  if (!s->nb_streams) return nullptr;
  
  AVDictionary **opts = static_cast<AVDictionary **>(
      av_calloc(s->nb_streams, sizeof(*opts)));
  if (!opts) return nullptr;
  
  for (int i = 0; i < static_cast<int>(s->nb_streams); i++) {
    opts[i] = filter_codec_opts(codec_opts, s->streams[i]->codecpar->codec_id,
                                s, s->streams[i], nullptr);
  }
  return opts;
}

// input_file.cc: open()
AVDictionary **opts = setup_find_stream_info_opts(ifmt_ctx_.get(), decoder_opts_);
unsigned int orig_nb_streams = ifmt_ctx_->nb_streams;

// RAII 方式释放 opts
auto opts_guard = std::shared_ptr<void>(nullptr, [&opts, orig_nb_streams](void *) {
  if (!opts) return;
  for (unsigned int i = 0; i < orig_nb_streams; i++) {
    av_dict_free(&opts[i]);
  }
  av_freep(&opts);
});

ret = avformat_find_stream_info(ifmt_ctx_.get(), opts);
```

**相关文件**：
- `pkg/cv/video/output_file.cc`

---

## 3. moov atom not found 问题

### 3.1 问题描述

生成的 MP4 文件无法正确播放，或者播放器显示"moov atom not found"错误。

**具体表现**：
- 输出视频文件无法获取时长
- 某些播放器无法播放
- 文件结构不完整

### 3.2 原因分析

1. **moov atom 位置问题**：MP4 文件的 moov atom（包含视频元数据）默认写在文件末尾。如果文件未正确关闭（如程序崩溃），moov atom 不会被写入。

2. **未调用 `av_write_trailer`**：正确关闭 MP4 文件需要调用 `av_write_trailer`，它会：
   - Flush 所有编码器缓冲区
   - 写入 moov atom
   - 完成文件结构

3. **faststart 标志未设置**：对于网络流媒体，moov atom 需要在文件开头才能实现快速播放。

### 3.3 解决方案

#### 3.3.1 设置 movflags +faststart

在 `OutputFile` 构造函数中设置：

```cpp
// output_file.cc
OutputFile::OutputFile() : av_class_(&output_file_class) {
  // 设置 movflags +faststart,将 moov atom 放在文件开头,避免无法检测时长
  // 参考: https://ffmpeg.org/ffmpeg-formats.html#mov_002c-mp4_002c-ismv
  av_dict_set(&opts_, "movflags", "+faststart", AV_DICT_APPEND);
  av_log(this, AV_LOG_WARNING, "OutputFile: set movflags=+faststart to opts_\n");
}
```

#### 3.3.2 确保正确 flush 和写入 trailer

```cpp
// output_file.cc
int OutputFile::flush() {
  int ret = 0;

  if (!header_written_) {
    return 0;
  }

  // 防止重复 flush
  if (flush_once_) {
    return 0;
  }
  flush_once_ = true;

  // Flush all encoders - 发送 null frame 清空编码器缓冲区
  for (unsigned int i = 0; i < output_streams_.size(); i++) {
    ret = flush_one_encoder(i);
    if (ret < 0) {
      return ret;
    }
  }

  // Write trailer - 写入 moov atom
  ret = of_write_trailer();
  if (ret < 0) {
    return ret;
  }

  return 0;
}

int OutputFile::of_write_trailer() {
  if (!header_written_) {
    av_log(this, AV_LOG_ERROR,
           "Nothing was written into output file\n");
    return AVERROR(EINVAL);
  }

  int ret = av_write_trailer(ofmt_ctx_.get());
  if (ret < 0) {
    av_log(this, AV_LOG_ERROR, "Error writing trailer: %s\n",
           av_err2str(ret));
    return ret;
  }

  return 0;
}
```

#### 3.3.3 析构时自动 flush

```cpp
OutputFile::~OutputFile() {
  // 在析构时自动 flush，确保 moov atom 被正确写入
  flush();
  // ...
}
```

**相关文件**：
- `pkg/cv/video/output_file.cc`
- `pkg/cv/video/output_file.h`

---

## 4. PTS（时间戳）问题

### 4.1 问题描述

输出视频的播放速度不正确（过快或过慢），或者播放时出现跳帧、卡顿。

**具体表现**：
- 视频播放速度异常
- 音视频不同步
- 编码器报告 PTS 单调递增警告

### 4.2 原因分析

**根本原因**：在解码后的帧中，`pts` 被设置了，但 `time_base` 没有被正确设置。

在 FFmpeg 6.x 中，`AVFrame::time_base` 字段需要显式设置，否则后续的时间基转换（`av_frame_rescale_ts`）无法正确工作。

**问题发生位置**：
1. `input_file.cc` 的 `decode_video` 函数：设置了 `decoded_frame->pts = best_effort_timestamp`，但没有设置 `time_base`
2. `input_file.cc` 的 `decode_audio` 函数：同样的问题

### 4.3 解决方案

#### 4.3.1 视频解码时设置 time_base

```cpp
// input_file.cc: decode_video()
if (best_effort_timestamp != AV_NOPTS_VALUE) {
  int64_t ts = av_rescale_q(decoded_frame->pts = best_effort_timestamp,
                            st->time_base, AV_TIME_BASE_Q);
  // 设置帧的 time_base，确保 PTS 与 time_base 一致
  decoded_frame->time_base = st->time_base;

  if (ts != AV_NOPTS_VALUE) {
    ist->next_pts_ = ist->pts_ = ts;
  }
}
```

#### 4.3.2 音频解码时设置 time_base

```cpp
// input_file.cc: decode_audio()
if (decoded_frame->pts != AV_NOPTS_VALUE) {
  decoded_frame->pts = av_rescale_delta(decoded_frame_tb, decoded_frame->pts,
                                        (AVRational){1, avctx->sample_rate},
                                        decoded_frame->nb_samples,
                                        &ist->filter_in_rescale_delta_last_,
                                        (AVRational){1, avctx->sample_rate});
  // 设置帧的 time_base，确保 PTS 与 time_base 一致
  decoded_frame->time_base = (AVRational){1, avctx->sample_rate};
}
```

#### 4.3.3 编码前正确转换时间基

```cpp
// output_file.cc: write_avframe()
auto &enc_ctx = ost->codec_ctx_;
if (enc_ctx && frame) {
  // 如果帧有有效的时间基，转换到编码器时间基
  if (frame->time_base.num > 0 && frame->time_base.den > 0 &&
      (frame->time_base.num != enc_ctx->time_base.num ||
       frame->time_base.den != enc_ctx->time_base.den)) {
    av_frame_rescale_ts(frame.get(), frame->time_base, enc_ctx->time_base);
    frame->time_base = enc_ctx->time_base;
  }
}
```

### 4.4 PTS 传递链路

正确的 PTS 传递链路如下：

```
1. 解码 (decode_video/decode_audio)
   └─ 设置 decoded_frame->pts = best_effort_timestamp
   └─ 设置 decoded_frame->time_base = st->time_base  // 关键！
   
2. Filter 处理 (send_frame_to_filters)
   └─ 帧进入 filter graph
   └─ reap_filters 获取过滤后帧
   └─ filtered_frame->time_base = av_buffersink_get_time_base(filter)
   
3. 存储到 Frame 结构体
   └─ raw_frame.pts = filtered_frame->pts
   └─ raw_frame.time_base = filtered_frame->time_base
   └─ raw_frame.frame = av_frame_clone(filtered_frame)
   
4. 编码 (write_avframe)
   └─ av_frame_rescale_ts(frame, frame->time_base, enc_ctx->time_base)
   └─ frame->time_base = enc_ctx->time_base
```

**相关文件**：
- `pkg/cv/video/input_file.cc`
- `pkg/cv/video/output_file.cc`
- `pkg/cv/video/output_filter.cc`

---

## 5. FPS（帧率）问题

### 5.1 问题描述

输出视频的帧率与预期不符，可能过高或过低。

### 5.2 原因分析

1. **编码器时间基设置不正确**：编码器的 `time_base` 应该是帧率的倒数（`1/fps`）
2. **从输入流获取的帧率信息不准确**：某些视频格式的 `avg_frame_rate` 或 `r_frame_rate` 可能为 0 或不准确
3. **Filter 时间基与编码器时间基不一致**

### 5.3 解决方案

#### 5.3.1 正确初始化编码器时间基

```cpp
// output_file.cc: init_encoder_time_base()
void OutputFile::init_encoder_time_base(
    const std::shared_ptr<OutputStream> &ost, AVRational default_time_base) {
  const auto &ist = ost->input_av_stream();
  auto enc_ctx = ost->codec_ctx_;

  // > 0 : 使用用户指定的 timebase
  if (ost->enc_timebase_.num > 0) {
    enc_ctx->time_base = ost->enc_timebase_;
    return;
  }
  
  // < 0 : 使用输入流的 timebase
  if (ost->enc_timebase_.num < 0) {
    if (ist && ist->time_base.num) {
      enc_ctx->time_base = ist->time_base;
      return;
    }
    av_log(this, AV_LOG_WARNING,
           "Input stream data not available, using default time base\n");
  }

  // = 0 : 使用默认 timebase (1/fps 或 1/sample_rate)
  enc_ctx->time_base = default_time_base;
}
```

#### 5.3.2 视频和音频的默认时间基

```cpp
// output_file.cc: init_output_stream_encode()
case AVMEDIA_TYPE_VIDEO:
  // 视频：time_base = 1/framerate
  init_encoder_time_base(ost, av_inv_q(ost->framerate_));
  if (filter_ctx && !(enc_ctx->time_base.num && enc_ctx->time_base.den)) {
    enc_ctx->time_base = av_buffersink_get_time_base(filter_ctx);
  }
  break;

case AVMEDIA_TYPE_AUDIO:
  // 音频：time_base = 1/sample_rate
  init_encoder_time_base(ost, av_make_q(1, enc_ctx->sample_rate));
  break;
```

#### 5.3.3 从输入流获取帧率

```cpp
// output_file.cc: create_streams()
if (ist && ist->codecpar) {
  // 优先使用 avg_frame_rate
  if (ist->avg_frame_rate.num > 0 && ist->avg_frame_rate.den > 0) {
    ost->framerate_ = ist->avg_frame_rate;
  } else if (ist->r_frame_rate.num > 0 && ist->r_frame_rate.den > 0) {
    ost->framerate_ = ist->r_frame_rate;
  } else {
    // 默认 25 fps
    ost->framerate_ = (AVRational){25, 1};
  }
}
```

**相关文件**：
- `pkg/cv/video/output_file.cc`
- `pkg/cv/video/output_stream.h`

---

## 6. 特殊视频流处理问题

### 6.1 问题描述

处理包含 Data/Subtitle/Attachment 等非音视频流的视频文件时，出现错误：
```
Failed to init input stream #0:2 -- Invalid argument
```

### 6.2 原因分析

某些视频文件（如 iOS 设备录制的视频）包含额外的元数据流：
```
Stream #0:0(und): Video: h264 ...
Stream #0:1(und): Audio: aac ...
Stream #0:2(und): Data: none (mebx / 0x7862656D)  // Core Media Metadata
Stream #0:3(und): Data: none (mebx / 0x7862656D)
Stream #0:4(und): Data: none (mebx / 0x7862656D)
```

对于 `AVMEDIA_TYPE_DATA`、`AVMEDIA_TYPE_SUBTITLE`、`AVMEDIA_TYPE_ATTACHMENT`、`AVMEDIA_TYPE_UNKNOWN` 这些类型的流：
1. 没有对应的解码器
2. 但 `decoding_needed_` 默认为 `true`
3. 导致 `init_input_stream()` 检查 `dec_` 为空时返回错误

### 6.3 解决方案

在 `choose_decoder` 函数中，对非 VIDEO/AUDIO 类型的流设置为 copy 模式：

```cpp
// input_file.cc: choose_decoder()
int InputFile::choose_decoder(const std::shared_ptr<InputStream> &ist,
                              const AVCodec *&codec) {
  std::string codec_name;
  auto &st = ist->st_;
  int ret = match_per_stream_opt(this, command_opts_, ifmt_ctx_.get(), st, "c",
                                 codec_name);
  if (ret != 0) return ret;

  // 对于非 VIDEO/AUDIO 类型的流，不支持解码，设置为 copy 模式
  switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
    case AVMEDIA_TYPE_AUDIO:
      break;
    default:
      /* no decoding supported for other media types */
      codec_name = "copy";
      break;
  }

  codec = nullptr;
  if (codec_name.empty() || codec_name == "copy") {
    if (codec_name != "copy") {
      codec = avcodec_find_decoder(st->codecpar->codec_id);
    }
    ist->decoding_needed_ = (codec_name != "copy");
  } else {
    // ... 其他逻辑
    ist->decoding_needed_ = true;
  }

  return 0;
}
```

**效果**：
- DATA/SUBTITLE/ATTACHMENT/UNKNOWN 类型的流 → `codec_name = "copy"` → `decoding_needed_ = false`
- 不会尝试打开不存在的解码器
- 转码后这些元数据流会被丢弃（正常行为）

**相关文件**：
- `pkg/cv/video/input_file.cc`

---

## 7. 自定义过滤器支持

### 7.1 功能描述

支持通过 `video_filter_spec_` 和 `audio_filter_spec_` 成员变量设置自定义过滤器。

### 7.2 实现方式

在 `InputFile` 类中添加过滤器成员变量：

```cpp
// input_file.h
class InputFile {
 public:
  // 自定义过滤器参数
  // 视频过滤器，如 "scale=1280:720", "transpose=1"
  std::string video_filter_spec_;
  // 音频过滤器，如 "aresample=44100", "volume=0.5"
  std::string audio_filter_spec_;
};
```

修改 `init_filters()` 使用自定义过滤器：

```cpp
// input_file.cc: init_filters()
int InputFile::init_filters() {
  for (unsigned int i = 0; i < ifmt_ctx_->nb_streams; i++) {
    std::string filter_spec;
    if (ifmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      if (!video_filter_spec_.empty()) {
        // 使用自定义视频过滤器
        filter_spec = bitexact_ ? "sws_flags=bitexact;" + video_filter_spec_ 
                                : video_filter_spec_;
      } else {
        // 默认 passthrough 过滤器
        filter_spec = bitexact_ ? "sws_flags=bitexact;null" : "null";
      }
    } else {
      if (!audio_filter_spec_.empty()) {
        filter_spec = audio_filter_spec_;
      } else {
        filter_spec = "anull";
      }
    }
    // ...
  }
}
```

### 7.3 常用过滤器

#### 视频过滤器

| 过滤器 | 说明 | 示例 |
|--------|------|------|
| `scale` | 缩放 | `scale=1280:720`, `scale=-1:720`(保持比例) |
| `transpose` | 旋转 | `transpose=1`(顺时针90°) |
| `crop` | 裁剪 | `crop=640:480:0:0` |
| `hflip/vflip` | 水平/垂直翻转 | `hflip` |
| `fps` | 改变帧率 | `fps=30` |
| `framestep` | 跳帧 | `framestep=2`(每2帧取1帧) |

#### 音频过滤器

| 过滤器 | 说明 | 示例 |
|--------|------|------|
| `aresample` | 重采样 | `aresample=44100` |
| `volume` | 调节音量 | `volume=0.5` |
| `atempo` | 调节速度 | `atempo=1.5` |

#### 跳帧过滤器

```bash
# 每 2 帧取 1 帧
VIDEO_FILTER="framestep=2"

# 每 5 帧取 1 帧
VIDEO_FILTER="framestep=5"

# 降低到指定帧率（如 10fps）
VIDEO_FILTER="fps=10"

# 跳帧 + 缩放组合
VIDEO_FILTER="framestep=2,scale=1280:720"
```

**相关文件**：
- `pkg/cv/video/input_file.h`
- `pkg/cv/video/input_file.cc`

---

## 8. 测试程序使用说明

### 8.1 基本用法

```bash
# 编译
make -j8

# 使用默认路径
./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 指定输入文件（输出自动生成为 video.copy.mp4）
VIDEO_INPUT=/path/to/video.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 同时指定输入和输出
VIDEO_INPUT=/path/to/input.mp4 VIDEO_OUTPUT=/path/to/output.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*
```

### 8.2 使用过滤器

```bash
# 缩放到 1280x720
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="scale=1280:720" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 旋转 90 度
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="transpose=1" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 多个过滤器组合
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="scale=1280:720,transpose=1" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 跳帧 - 每 2 帧取 1 帧
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="framestep=2" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 降低到 10fps
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="fps=10" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 音频重采样
VIDEO_INPUT=/path/to/input.mp4 AUDIO_FILTER="aresample=44100" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 同时使用视频和音频过滤器
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="scale=1280:720" AUDIO_FILTER="volume=0.5" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*
```

### 8.3 环境变量说明

| 环境变量 | 说明 | 默认值 |
|---------|------|--------|
| `VIDEO_INPUT` | 输入视频路径 | `./testdata/sce_video.mp4` |
| `VIDEO_OUTPUT` | 输出视频路径 | `{input}.copy.{ext}` |
| `VIDEO_FILTER` | 视频过滤器 | 无（passthrough） |
| `AUDIO_FILTER` | 音频过滤器 | 无（passthrough） |

**相关文件**：
- `test/pkg/test_cv_video.cc`

---

## 9. FFmpeg 核心概念

### 9.1 AVPacket vs AVFrame

AVPacket 和 AVFrame 是 FFmpeg 中两个核心数据结构，代表视频处理流水线中不同阶段的数据。

#### 9.1.1 基本对比

| 特性 | AVPacket | AVFrame |
|------|----------|---------|
| **数据状态** | 压缩数据（编码后） | 原始数据（解码后） |
| **存储内容** | 编码的比特流 | 像素/采样数据 |
| **数据大小** | 小（压缩） | 大（未压缩） |
| **来源** | 从容器读取 / 编码器输出 | 解码器输出 / 待编码输入 |

#### 9.1.2 数据流位置

```
┌─────────────┐    demux     ┌─────────────┐    decode    ┌─────────────┐
│  容器文件    │ ──────────> │  AVPacket   │ ──────────> │  AVFrame    │
│ (mp4/mkv)   │             │  (压缩数据)  │             │  (原始数据)  │
└─────────────┘             └─────────────┘             └─────────────┘
                                   ▲                          │
                                   │         encode           │
                                   └──────────────────────────┘
```

#### 9.1.3 结构对比

**AVPacket（压缩数据包）**：
```c
typedef struct AVPacket {
    uint8_t *data;      // 压缩数据指针（H.264/AAC等编码数据）
    int size;           // 数据大小（通常几KB~几百KB）
    int64_t pts;        // 显示时间戳
    int64_t dts;        // 解码时间戳（B帧时 dts != pts）
    int stream_index;   // 所属流索引
    int flags;          // 关键帧标志等
    int64_t duration;   // 持续时间
} AVPacket;
```

**AVFrame（原始帧）**：
```c
typedef struct AVFrame {
    // 视频
    uint8_t *data[8];   // 像素数据（YUV各平面或RGB）
    int linesize[8];    // 每行字节数
    int width, height;  // 分辨率
    int format;         // 像素格式（AV_PIX_FMT_YUV420P等）
    
    // 音频
    uint8_t **extended_data;  // 采样数据
    int nb_samples;           // 采样数
    int sample_rate;          // 采样率
    int channels;             // 声道数
    
    // 通用
    int64_t pts;              // 显示时间戳
    AVRational time_base;     // 时间基
} AVFrame;
```

#### 9.1.4 在项目中的使用

```cpp
// InputFile::process_input_packet() - 处理 AVPacket
int InputFile::process_input_packet(AVPacketPtr pkt) {
    // pkt 是从文件读取的压缩数据
    return decode(ist, pkt.get(), ...);  // 解码成 AVFrame
}

// decode_video() - AVPacket -> AVFrame
int InputFile::decode_video(InputStream* ist, AVPacket* pkt, ...) {
    avcodec_send_packet(dec, pkt);           // 发送压缩包
    avcodec_receive_frame(dec, frame);       // 接收原始帧
    // frame 现在包含解码后的像素数据
}

// OutputFile::encode() - AVFrame -> AVPacket
int OutputFile::encode(OutputStream* ost, AVFrame* frame) {
    avcodec_send_frame(enc, frame);          // 发送原始帧
    avcodec_receive_packet(enc, pkt);        // 接收压缩包
    // pkt 现在包含编码后的比特流
}
```

#### 9.1.5 典型数据大小对比

| 类型 | AVPacket | AVFrame |
|------|----------|---------|
| 1080p 视频帧 | ~50KB (H.264) | ~3MB (YUV420P) |
| 音频 1024 采样 | ~1KB (AAC) | ~8KB (PCM 16bit stereo) |

**核心区别**：AVPacket 是传输/存储用的压缩格式，AVFrame 是处理用的原始格式。解码是 Packet→Frame，编码是 Frame→Packet。

---

## 10. 音视频同步机制

### 10.1 问题背景

在转码过程中，音频和视频是分开处理的，需要确保输出文件中音视频保持同步。

### 10.2 同步保证机制

项目通过以下几个关键机制保证音视频同步：

#### 10.2.1 统一时间基转换

```
┌──────────────────┐    ┌──────────────────┐    ┌──────────────────┐
│ 输入流 time_base │ -> │ 编码器 time_base │ -> │ 输出流 time_base │
│  (解码阶段)      │    │   (编码阶段)     │    │   (写入阶段)     │
└──────────────────┘    └──────────────────┘    └──────────────────┘
```

关键代码在 `output_file.cc`：

```cpp
// 写入前转换帧时间基到编码器时间基
if (frame->time_base.num > 0 && frame->time_base.den > 0 &&
    (frame->time_base.num != enc_ctx->time_base.num ||
     frame->time_base.den != enc_ctx->time_base.den)) {
  av_frame_rescale_ts(frame.get(), frame->time_base, enc_ctx->time_base);
  frame->time_base = enc_ctx->time_base;
}

// 写入前转换 packet 时间基到输出流时间基
av_packet_rescale_ts(pkt, pkt->time_base, st->time_base);
pkt->time_base = st->time_base;
```

#### 10.2.2 交织写入（Interleaved Write）

```cpp
// output_file.cc: of_write_packet()
// 使用 av_interleaved_write_frame 而非 av_write_frame
ret = av_interleaved_write_frame(ofmt_ctx_.get(), pkt);
```

`av_interleaved_write_frame` 会：
- 内部维护音频和视频的 packet 队列
- 按 **DTS 升序** 自动排序后再写入
- 确保输出文件中音视频 packet 交错存储

#### 10.2.3 DTS 单调递增保证

```cpp
// output_file.cc: of_write_packet()
if (!(s->oformat->flags & AVFMT_NOTIMESTAMPS)) {
    // 检查 DTS > PTS 的非法情况
    if (pkt->dts != AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE &&
        pkt->dts > pkt->pts) {
        // 自动修正
        pkt->pts = pkt->dts = ...;
    }
    
    // 检查 DTS 非单调递增
    if (pkt->dts < max) {
        // 修正为 max，保证单调递增
        if (pkt->pts >= pkt->dts) {
            pkt->pts = FFMAX(pkt->pts, max);
        }
        pkt->dts = max;
    }
}

// 记录上次 DTS 用于下次比较
ost->last_mux_dts_ = pkt->dts;
```

#### 10.2.4 音视频时间基设置

| 流类型 | 编码器 time_base 设置 |
|--------|---------------------|
| 视频 | `1 / framerate`（如 1/25） |
| 音频 | `1 / sample_rate`（如 1/44100） |

```cpp
// output_file.cc: init_output_stream_encode()
// 音频编码器时间基
init_encoder_time_base(ost, av_make_q(1, enc_ctx->sample_rate));

// 视频编码器时间基
enc_ctx->time_base = av_inv_q(ost->codec_ctx_->framerate);
```

### 10.3 同步流程图

```
┌─────────┐                              ┌─────────┐
│ 视频流  │                              │ 音频流  │
└────┬────┘                              └────┬────┘
     │ encode                                 │ encode
     ▼                                        ▼
┌─────────────────┐                   ┌─────────────────┐
│ AVPacket        │                   │ AVPacket        │
│ pts=100, dts=100│                   │ pts=4410,dts=4410│
└────────┬────────┘                   └────────┬────────┘
         │ rescale_ts                          │ rescale_ts
         │ (enc_tb -> st_tb)                   │ (enc_tb -> st_tb)
         ▼                                     ▼
    ┌────────────────────────────────────────────┐
    │        av_interleaved_write_frame          │
    │  ┌──────────────────────────────────────┐  │
    │  │  内部队列（按 DTS 排序）              │  │
    │  │  Audio DTS=0.1s, Video DTS=0.1s      │  │
    │  └──────────────────────────────────────┘  │
    └─────────────────────┬──────────────────────┘
                          │
                          ▼
                   ┌─────────────┐
                   │  输出文件    │
                   │ (DTS 升序)   │
                   └─────────────┘
```

### 10.4 测试代码中的同步

在 `test_cv_video.cc` 中的写入方式：

```cpp
while (!finished) {
    // 1. 读取帧（音视频分开存储）
    input_file.read_frames(video_frames, audio_frames, 8, finished);
    
    // 2. 先写视频帧
    output_file.write_frames(video_frames);
    
    // 3. 再写音频帧
    output_file.write_frames(audio_frames);
}
```

虽然写入顺序是先视频后音频，但由于 `av_interleaved_write_frame` 内部会重新按 DTS 排序，**最终输出文件的音视频是同步的**。

### 10.5 同步机制总结

| 机制 | 作用 |
|------|------|
| `av_packet_rescale_ts` | 统一时间基，确保 PTS/DTS 可比较 |
| `av_interleaved_write_frame` | 自动交织排序，按 DTS 顺序写入 |
| DTS 单调递增检查 | 修正乱序/回退的时间戳 |
| `last_mux_dts_` 追踪 | 记录每个流的最后 DTS，用于单调性检查 |

**相关文件**：
- `pkg/cv/video/output_file.cc`
- `pkg/cv/video/input_file.cc`
- `test/pkg/test_cv_video.cc`

---

## 总结

| 问题类型 | 根本原因 | 关键修复点 |
|---------|---------|-----------|
| 编译问题 | FFmpeg C 头文件在 C++ 中需要 `extern "C"` | 正确包裹头文件，使用自定义删除器 |
| CMake 配置 | protobuf 版本冲突、gtest 集成不完整、ABI 兼容性 | 统一 protobuf 3.6.1、修复 gtest 集成、添加 `_GLIBCXX_USE_CXX11_ABI=0` |
| Core Dump | 空指针访问、资源释放顺序错误、指针被修改 | 空指针检查，正确的析构顺序，`avformat_open_input` 后创建 `shared_ptr` |
| moov atom | 未正确关闭文件 | 设置 `movflags +faststart`，确保调用 `flush()` 和 `av_write_trailer()` |
| PTS 问题 | 解码后帧的 `time_base` 未设置 | 在 `decode_video/decode_audio` 中设置 `decoded_frame->time_base` |
| FPS 问题 | 编码器时间基设置错误 | 使用 `init_encoder_time_base` 正确设置 `enc_ctx->time_base = av_inv_q(framerate)` |
| 特殊流处理 | 非音视频流无解码器但 `decoding_needed_=true` | 在 `choose_decoder` 中对非音视频流设置 `codec_name="copy"` |
| 过滤器支持 | 需要自定义视频/音频处理 | 添加 `video_filter_spec_` 和 `audio_filter_spec_` 成员变量 |
| 音视频同步 | 音视频分开处理后需保证同步 | `av_interleaved_write_frame` 按 DTS 排序 + 时间基转换 + DTS 单调递增检查 |
| CUDA 硬件编解码 | GPU 设备初始化、环境权限问题 | 设置 `gpu` 选项、自动回退机制、检查系统环境 |

## 核心概念速查

| 概念 | 说明 |
|------|------|
| AVPacket | 压缩数据（编码后），用于存储/传输 |
| AVFrame | 原始数据（解码后），用于处理 |
| PTS | 显示时间戳（Presentation Time Stamp） |
| DTS | 解码时间戳（Decoding Time Stamp），B帧时 DTS ≠ PTS |
| time_base | 时间基，PTS/DTS 的单位（如 1/25 表示每单位 40ms） |
| `av_interleaved_write_frame` | 交织写入，内部按 DTS 排序后写入文件 |

## 参考资料

- [FFmpeg Formats - mov, mp4, ismv](https://ffmpeg.org/ffmpeg-formats.html#mov_002c-mp4_002c-ismv)
- [FFmpeg Time Base and PTS](https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html)
- [FFmpeg Source Code - fftools/ffmpeg.c](https://github.com/FFmpeg/FFmpeg/blob/master/fftools/ffmpeg.c)

---

## 11. CUDA 硬件编解码支持

### 11.1 功能概述

支持使用 NVIDIA GPU 进行硬件加速的视频编解码：

| 功能 | 硬件加速器 | 支持的编解码器 |
|------|-----------|---------------|
| 解码 | CUVID (NVDEC) | h264_cuvid, hevc_cuvid, av1_cuvid, mjpeg_cuvid, mpeg1/2/4_cuvid, vp8/9_cuvid |
| 编码 | NVENC | h264_nvenc, hevc_nvenc, av1_nvenc |

### 11.2 实现架构

#### 11.2.1 核心参数

```cpp
// gpu_id_ 参数含义
// >= 0 : 使用指定 GPU 进行硬件编解码
// < 0  : 使用软件编解码（默认）
int64_t gpu_id_ = -1;

// 自动回退开关
// true  : 硬件编解码失败时自动切换到软件编解码
// false : 硬件编解码失败时直接返回错误
bool auto_switch_to_soft_codec_ = true;
```

#### 11.2.2 修改的文件

| 文件 | 修改内容 |
|------|---------|
| `ffmpeg_hw.h/cc` | GPU 检测和编解码器名称映射函数 |
| `input_file.h/cc` | 添加 `gpu_id_` 成员，CUDA 解码器选择逻辑 |
| `input_stream.h/cc` | 添加 `gpu_id_`，CUDA 解码器初始化和自动回退 |
| `output_file.h/cc` | 添加 `gpu_id_`，NVENC 编码器选择和 GPU ID 设置 |
| `test_cv_video.cc` | 添加 `GPU_ID` 环境变量支持 |

### 11.3 使用方法

```bash
# 软件编解码（默认）
./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# GPU 硬件加速
GPU_ID=0 VIDEO_INPUT=/path/to/input.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# GPU + 滤镜
GPU_ID=0 VIDEO_FILTER="scale=1280:720" VIDEO_INPUT=/path/to/input.mp4 ./output/bin/kingfisher_base_test
```

### 11.4 遇到的问题及解决方案

#### 11.4.1 CUDA_ERROR_OUT_OF_MEMORY 错误

**问题描述**：
```
[h264_cuvid @ 0x...] ctx->cvdl->cuvidGetDecoderCaps(&ctx->caps8) failed -> CUDA_ERROR_OUT_OF_MEMORY: out of memory
```

**原因分析**：
这个错误通常 **不是真正的显存不足**，而是 CUDA 解码器初始化时没有正确设置 GPU 设备。

**解决方案**：
在 `avcodec_open2()` 之前为 CUDA 解码器设置必要的选项：

```cpp
// input_stream.cc: init_input_stream()
if (is_prefer_gpu(gpu_id_) && is_cuda_decoder(dec_->name)) {
  // 设置 GPU ID
  av_dict_set_int(&decoder_opts_, "gpu", gpu_id_, 0);
  // 设置去隔行模式
  av_dict_set(&decoder_opts_, "deint", "adaptive", 0);
  // 去隔行时不复制第二场
  av_dict_set_int(&decoder_opts_, "drop_second_field", 1, 0);
}
```

#### 11.4.2 av_err2str 在 C++ 中编译错误

**问题描述**：
```
error: taking address of temporary array
```

**原因分析**：
`av_err2str` 宏在 C++ 中使用临时数组会导致编译错误。

**解决方案**：
使用 `av_strerror()` 替代：

```cpp
// 错误写法
av_log(nullptr, AV_LOG_WARNING, "Error: %s\n", av_err2str(ret));

// 正确写法
char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
av_strerror(ret, errbuf, sizeof(errbuf));
av_log(nullptr, AV_LOG_WARNING, "Error: %s\n", errbuf);
```

#### 11.4.3 shared_ptr::get() 不能作为左值

**问题描述**：
```
error: lvalue required as unary '&' operand
avcodec_free_context(&codec_ctx_.get());
```

**原因分析**：
`shared_ptr::get()` 返回的是右值，不能取地址传给需要指针的指针的函数。

**解决方案**：
使用 `reset()` 释放 shared_ptr 管理的资源：

```cpp
// 错误写法
avcodec_free_context(&codec_ctx_.get());

// 正确写法
codec_ctx_.reset();
```

#### 11.4.4 CUDA_ERROR_NOT_PERMITTED 错误

**问题描述**：
```
[h264_nvenc @ 0x...] cuCtxCreate(&ctx->cu_context_internal, 0, cu_device) failed -> CUDA_ERROR_NOT_PERMITTED: operation not permitted
[h264_nvenc @ 0x...] No NVENC capable devices found
```

**原因分析**：
这是 **系统环境问题**，不是代码问题。常见于：

1. **vQGPU 虚拟化环境** - GPU 的 NVENC/NVDEC 功能被云平台策略禁用
2. **NVIDIA 设备节点缺失** - `/dev/nvidia*` 设备文件不完整
3. **驱动模块未加载** - `nvidia_uvm` 等模块未正确加载

**诊断方法**：

```bash
# 1. 检查 GPU 设备
nvidia-smi -L

# 2. 检查设备节点
ls -la /dev/nvidia*

# 3. 检查驱动模块
lsmod | grep nvidia

# 4. 测试 CUDA 初始化
python3 -c "import ctypes; cuda = ctypes.CDLL('libcuda.so.1'); print('cuInit:', cuda.cuInit(0))"

# 5. 测试 FFmpeg 硬件加速
ffmpeg -y -i input.mp4 -c:v h264_nvenc -frames:v 10 test.mp4
```

**解决方案**：

| 问题 | 解决方案 |
|------|---------|
| 设备节点缺失 | `sudo mknod -m 666 /dev/nvidia1 c 195 1` |
| nvidia_uvm 未加载 | `sudo insmod /lib/modules/.../nvidia_uvm.ko` |
| vQGPU 功能受限 | 联系云平台管理员开通 NVENC/NVDEC 功能，或使用专用 GPU 实例 |
| 驱动版本不匹配 | 升级/重装 NVIDIA 驱动 |

### 11.5 自动回退机制

当硬件编解码不可用时，代码会自动回退到软件编解码。

#### 11.5.1 CUVID 解码器回退

```cpp
// input_stream.cc
int ret = avcodec_open2(codec_ctx_.get(), dec_, &decoder_opts_);
if (ret < 0) {
  // 硬件解码失败，尝试回退到软件解码
  if (using_cuda && auto_switch_to_soft_codec_) {
    const AVCodec *sw_dec = avcodec_find_decoder(st_->codecpar->codec_id);
    if (sw_dec) {
      codec_ctx_.reset();
      codec_ctx_ = std::shared_ptr<AVCodecContext>(
          avcodec_alloc_context3(sw_dec), ...);
      // ... 重新配置并打开软件解码器
    }
  }
}
```

日志输出示例：
```
Using CUDA decoder [h264_cuvid] for stream #0:0 (codec_id=27, gpu_id=0)
Setting GPU ID 0 for CUDA decoder [h264_cuvid] stream #0:0
CUDA decoder [h264_cuvid] failed for stream #0:0, falling back to software decoder: ...
Successfully switched to software decoder [h264] for stream #0:0
```

#### 11.5.2 NVENC 编码器回退

**问题描述**：

当 NVENC 硬件编码器初始化失败时，之前没有自动回退机制，导致整个转码失败：

```
[h264_nvenc @ 0x...] dl_fn->cuda_dl->cuCtxCreate(&ctx->cu_context_internal, 0, cu_device) failed -> CUDA_ERROR_INVALID_VALUE: invalid argument
[h264_nvenc @ 0x...] No capable devices found
Error while opening encoder for output stream #0:0 -- Generic error in an external library
```

**解决方案**：

在 `output_file.cc` 的 `init_output_stream` 函数中添加 NVENC 编码器自动回退机制：

```cpp
// output_file.cc: init_output_stream()
bool using_nvenc = is_prefer_gpu(gpu_id_) && is_nvenc_encoder(ost->codec_ctx_->codec->name);
if (using_nvenc) {
  av_dict_set_int(&ost->codec_opts_, "gpu", gpu_id_, AV_DICT_DONT_OVERWRITE);
}

ret = avcodec_open2(ost->codec_ctx_.get(), codec, &ost->codec_opts_);
if (ret < 0) {
  // NVENC 编码器初始化失败，尝试回退到软件编码器
  if (using_nvenc && auto_switch_to_soft_codec_) {
    char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(ret, errbuf, sizeof(errbuf));
    av_log(this, AV_LOG_WARNING,
           "NVENC encoder [%s] failed for stream #%d:%d, falling back to "
           "software encoder: %s\n",
           codec->name, file_index_, ost->stream_index_, errbuf);

    // 查找软件编码器
    const AVCodec *sw_enc = avcodec_find_encoder(st->codecpar->codec_id);
    if (sw_enc && !is_nvenc_encoder(sw_enc->name)) {
      // 重新分配 codec context
      ost->codec_ctx_.reset();
      ost->codec_ctx_ = std::shared_ptr<AVCodecContext>(
          avcodec_alloc_context3(sw_enc),
          [](AVCodecContext *ctx) { avcodec_free_context(&ctx); });

      // 重新设置编码器参数
      AVCodecContext *enc_ctx = ost->codec_ctx_.get();
      enc_ctx->codec_type = st->codecpar->codec_type;
      enc_ctx->width = st->codecpar->width;
      enc_ctx->height = st->codecpar->height;
      enc_ctx->pix_fmt = static_cast<AVPixelFormat>(st->codecpar->format);
      enc_ctx->framerate = ost->framerate_;
      enc_ctx->time_base = av_inv_q(ost->framerate_);

      // 清除 NVENC 相关选项
      av_dict_set(&ost->codec_opts_, "gpu", nullptr, 0);

      // 设置软件编码器选项
      if (enc_ctx->codec_id == AV_CODEC_ID_H264 || enc_ctx->codec_id == AV_CODEC_ID_HEVC) {
        av_dict_set(&ost->codec_opts_, "tune", "zerolatency", AV_DICT_DONT_OVERWRITE);
      }

      // 重新尝试打开编码器
      ret = avcodec_open2(ost->codec_ctx_.get(), sw_enc, &ost->codec_opts_);
      if (ret >= 0) {
        av_log(this, AV_LOG_INFO,
               "Successfully switched to software encoder [%s] for stream #%d:%d\n",
               sw_enc->name, file_index_, ost->stream_index_);
      }
    }
  }
}
```

**回退流程**：

1. 检测是否正在使用 NVENC 编码器
2. `avcodec_open2` 失败后检查是否启用自动回退
3. 查找对应的软件编码器（如 `libx264`）
4. 重新分配 `AVCodecContext`
5. 复制视频参数（宽高、像素格式、帧率、时间基）
6. 清除 NVENC 相关选项（`gpu`）
7. 设置软件编码器选项（`tune=zerolatency`）
8. 重新尝试打开编码器

**日志输出示例**：

```
Using NVENC encoder [h264_nvenc] for stream #0:0 (codec_id=27, gpu_id=0)
Setting GPU ID 0 for NVENC encoder [h264_nvenc] stream #0:0
NVENC encoder [h264_nvenc] failed for stream #0:0, falling back to software encoder: Generic error in an external library
Successfully switched to software encoder [libx264] for stream #0:0
```

### 11.6 环境要求

| 组件 | 最低版本 | 推荐版本 |
|------|---------|---------|
| NVIDIA 驱动 | 418.x | 515.x+ |
| CUDA | 10.0 | 11.7+ |
| FFmpeg | 4.2 | 5.1+ |
| GPU | Kepler (GTX 600+) | Turing/Ampere (RTX 20/30/40) |

**注意**：FFmpeg 4.2 的 CUDA 支持有限，建议使用 FFmpeg 5.x 以获得更好的兼容性。

### 11.7 相关文件

- `pkg/cv/video/ffmpeg_hw.h` - GPU 检测函数声明
- `pkg/cv/video/ffmpeg_hw.cc` - GPU 检测函数实现
- `pkg/cv/video/input_file.h/cc` - CUDA 解码器选择
- `pkg/cv/video/input_stream.h/cc` - CUDA 解码器初始化
- `pkg/cv/video/output_file.h/cc` - NVENC 编码器选择
- `test/pkg/test_cv_video.cc` - GPU_ID 环境变量支持
