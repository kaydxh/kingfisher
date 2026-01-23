# Kingfisher 视频处理模块问题解决总结

本文档总结了 Kingfisher 项目中视频处理模块（`pkg/cv/video`）在开发过程中遇到的所有问题及其解决方案。

---

## 目录

1. [编译问题](#1-编译问题)
2. [Core Dump 问题](#2-core-dump-问题)
3. [moov atom not found 问题](#3-moov-atom-not-found-问题)
4. [PTS（时间戳）问题](#4-pts时间戳问题)
5. [FPS（帧率）问题](#5-fps帧率问题)

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

## 总结

| 问题类型 | 根本原因 | 关键修复点 |
|---------|---------|-----------|
| 编译问题 | FFmpeg C 头文件在 C++ 中需要 `extern "C"` | 正确包裹头文件，使用自定义删除器 |
| Core Dump | 空指针访问、资源释放顺序错误 | 空指针检查，正确的析构顺序 |
| moov atom | 未正确关闭文件 | 设置 `movflags +faststart`，确保调用 `flush()` 和 `av_write_trailer()` |
| PTS 问题 | 解码后帧的 `time_base` 未设置 | 在 `decode_video/decode_audio` 中设置 `decoded_frame->time_base` |
| FPS 问题 | 编码器时间基设置错误 | 使用 `init_encoder_time_base` 正确设置 `enc_ctx->time_base = av_inv_q(framerate)` |

## 参考资料

- [FFmpeg Formats - mov, mp4, ismv](https://ffmpeg.org/ffmpeg-formats.html#mov_002c-mp4_002c-ismv)
- [FFmpeg Time Base and PTS](https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html)
- [FFmpeg Source Code - fftools/ffmpeg.c](https://github.com/FFmpeg/FFmpeg/blob/master/fftools/ffmpeg.c)
