# Kingfisher 视频编解码模块技术文档

本文档详细介绍 Kingfisher 项目中视频处理模块（`pkg/cv/video`）的架构设计、核心类、数据流和使用方式。

---

## 目录

1. [模块概述](#1-模块概述)
2. [整体架构](#2-整体架构)
3. [核心类设计](#3-核心类设计)
4. [数据流分析](#4-数据流分析)
5. [解码流程详解](#5-解码流程详解)
6. [编码流程详解](#6-编码流程详解)
7. [Filter 过滤器系统](#7-filter-过滤器系统)
8. [关键数据结构](#8-关键数据结构)
9. [使用示例](#9-使用示例)

---

## 1. 模块概述

### 1.1 功能特性

- **视频解码**：支持多种视频格式的解码（H.264、H.265、VP9 等）
- **视频编码**：支持重新编码输出
- **音频处理**：同步处理音频流
- **Filter 链**：支持视频/音频过滤器（缩放、旋转、裁剪等）
- **流复制**：支持无损流复制模式
- **批量读取**：支持按批次读取解码帧

### 1.2 文件结构

```
pkg/cv/video/
├── input_file.h/cc          # 输入文件处理（解码入口）
├── output_file.h/cc         # 输出文件处理（编码入口）
├── input_stream.h/cc        # 输入流管理
├── output_stream.h/cc       # 输出流管理
├── stream.h/cc              # 流基类
├── ffmpeg_filter.h/cc       # FilterGraph 管理
├── input_filter.h/cc        # 输入过滤器
├── output_filter.h/cc       # 输出过滤器
├── ffmpeg_types.h           # 数据类型定义
├── ffmpeg_utils.h/cc        # 工具函数
├── ffmpeg_error.h           # 错误处理
└── ffmpeg_hw.h/cc           # 硬件加速（预留）
```

---

## 2. 整体架构

### 2.1 架构图

```mermaid
graph TB
    subgraph "输入层 Input Layer"
        IF[InputFile<br/>输入文件管理]
        IS[InputStream<br/>输入流]
    end

    subgraph "过滤层 Filter Layer"
        FG[FilterGraph<br/>过滤器图]
        InF[InputFilter<br/>输入过滤器]
        OutF[OutputFilter<br/>输出过滤器]
    end

    subgraph "输出层 Output Layer"
        OF[OutputFile<br/>输出文件管理]
        OS[OutputStream<br/>输出流]
    end

    subgraph "FFmpeg 核心"
        AVF[AVFormatContext<br/>格式上下文]
        AVC[AVCodecContext<br/>编解码上下文]
        AVP[AVPacket<br/>压缩数据包]
        AVFr[AVFrame<br/>原始帧数据]
    end

    IF --> IS
    IS --> InF
    InF --> FG
    FG --> OutF
    OutF --> OS
    OS --> OF

    AVF -.-> IF
    AVF -.-> OF
    AVC -.-> IS
    AVC -.-> OS
    AVP -.-> IS
    AVP -.-> OS
    AVFr -.-> InF
    AVFr -.-> OutF
```

### 2.2 类继承关系

```mermaid
classDiagram
    class Stream {
        +weak_ptr~AVFormatContext~ fmt_ctx_
        +AVStream* st_
        +shared_ptr~AVCodecContext~ codec_ctx_
        +shared_ptr~AVFrame~ frame_
        +shared_ptr~AVPacket~ pkt_
        +av_stream() AVStream*
    }

    class InputStream {
        +bool decoding_needed_
        +const AVCodec* dec_
        +shared_ptr~FilterGraph~ ifilt_
        +init_input_stream() int
    }

    class OutputStream {
        +bool encoding_needed_
        +shared_ptr~FilterGraph~ ofilt_
        +OutputFilter* filter_
    }

    class FilterGraph {
        +shared_ptr~AVFilterGraph~ filter_graph_
        +vector~InputFilter~ inputs_
        +vector~OutputFilter~ outputs_
        +configure_filtergraph() int
        +reap_filters() int
    }

    class InputFilter {
        +AVFilterContext* filter_
        +ifilter_send_frame() int
    }

    class OutputFilter {
        +AVFilterContext* filter_
        +reap_filters() int
    }

    Stream <|-- InputStream
    Stream <|-- OutputStream
    FilterGraph *-- InputFilter
    FilterGraph *-- OutputFilter
    InputStream o-- FilterGraph
    OutputStream o-- FilterGraph
```

---

## 3. 核心类设计

### 3.1 InputFile - 输入文件管理

**职责**：打开输入文件、管理输入流、读取和解码数据包

```cpp
class InputFile {
public:
    // 打开输入文件
    int open(const std::string &filename, FormatContext &format_ctx);
    
    // 批量读取解码帧
    int read_frames(std::vector<Frame> &video_frames,
                    std::vector<Frame> &audio_frames, 
                    int32_t batch_size, bool &finished);

private:
    // 添加输入流
    int add_input_streams();
    // 选择解码器
    int choose_decoder(const std::shared_ptr<InputStream> &ist, const AVCodec *&codec);
    // 处理输入数据包
    int process_input_packet(const std::shared_ptr<InputStream> &ist, AVPacket *pkt, int no_eof);
    // 视频解码
    int decode_video(const std::shared_ptr<InputStream> &ist, AVPacket *pkt, int eof, 
                     bool &got_output, int64_t &duration_pts, bool &decode_failed);
    // 音频解码
    int decode_audio(const std::shared_ptr<InputStream> &ist, AVPacket *pkt,
                     bool &got_output, bool &decode_failed);
    // 初始化过滤器
    int init_filters();

public:
    std::shared_ptr<AVFormatContext> ifmt_ctx_;           // 输入格式上下文
    std::vector<std::shared_ptr<InputStream>> input_streams_; // 输入流列表
    std::string video_filter_spec_;  // 自定义视频过滤器
    std::string audio_filter_spec_;  // 自定义音频过滤器
};
```

### 3.2 OutputFile - 输出文件管理

**职责**：创建输出文件、管理输出流、编码和写入数据

```cpp
class OutputFile {
public:
    // 打开输出文件
    int open(const std::string &filename, FormatContext &format_ctx);
    // 写入帧数据
    int write_frames(const std::vector<Frame> &raw_frames);
    // 刷新缓冲区
    int flush();

private:
    // 创建输出流
    int create_streams(const FormatContext &format_ctx);
    // 选择编码器
    int choose_encoder(const std::shared_ptr<OutputStream> &ost, const AVCodec *&codec);
    // 初始化输出流编码
    int init_output_stream_encode(const std::shared_ptr<OutputStream> &ost, AVFrame *frame);
    // 编码帧
    int of_encode_frame(const std::shared_ptr<OutputStream> &ost, const std::shared_ptr<AVFrame> &frame);
    // 写入数据包
    int of_write_packet(const std::shared_ptr<OutputStream> &ost, AVPacket *pkt);

public:
    std::shared_ptr<AVFormatContext> ofmt_ctx_;           // 输出格式上下文
    std::vector<std::shared_ptr<OutputStream>> output_streams_; // 输出流列表
};
```

### 3.3 FilterGraph - 过滤器图

**职责**：管理 FFmpeg 过滤器链，处理视频/音频变换

```cpp
class FilterGraph : public std::enable_shared_from_this<FilterGraph> {
public:
    // 配置过滤器图
    int configure_filtergraph();
    // 初始化简单过滤器图
    int init_simple_filtergraph();
    // 发送帧到过滤器
    int send_frame_to_filters(const std::shared_ptr<AVFrame> &decoded_frame);
    // 收集过滤后的帧
    int reap_filters(std::vector<std::shared_ptr<AVFrame>> &filtered_frames, bool need_filtered_frames);
    // 发送 EOF
    int send_filter_eof(int64_t pts);

public:
    std::shared_ptr<AVFilterGraph> filter_graph_;
    std::vector<std::shared_ptr<InputFilter>> inputs_;
    std::vector<std::shared_ptr<OutputFilter>> outputs_;
    std::string graph_desc_;  // 过滤器描述字符串
};
```

---

## 4. 数据流分析

### 4.1 完整数据流图

```mermaid
flowchart TB
    subgraph "输入阶段"
        A[视频文件] --> B[avformat_open_input]
        B --> C[avformat_find_stream_info]
        C --> D[InputFile::add_input_streams]
        D --> E[为每个流创建 InputStream]
    end

    subgraph "解码阶段"
        F[av_read_frame] --> G{流类型?}
        G -->|Video| H[decode_video]
        G -->|Audio| I[decode_audio]
        H --> J[avcodec_send_packet]
        I --> J
        J --> K[avcodec_receive_frame]
        K --> L[AVFrame 原始帧]
    end

    subgraph "过滤阶段"
        L --> M[send_frame_to_filters]
        M --> N[av_buffersrc_add_frame]
        N --> O[FilterGraph 处理]
        O --> P[av_buffersink_get_frame]
        P --> Q[过滤后 AVFrame]
    end

    subgraph "编码阶段"
        Q --> R[of_encode_frame]
        R --> S[avcodec_send_frame]
        S --> T[avcodec_receive_packet]
        T --> U[AVPacket 压缩数据]
    end

    subgraph "输出阶段"
        U --> V[of_write_packet]
        V --> W[av_interleaved_write_frame]
        W --> X[输出文件]
    end

    E --> F
```

### 4.2 数据结构转换流程

```mermaid
sequenceDiagram
    participant File as 视频文件
    participant Demux as Demuxer
    participant Dec as Decoder
    participant Filter as Filter
    participant Enc as Encoder
    participant Mux as Muxer
    participant Out as 输出文件

    File->>Demux: 读取文件
    Demux->>Dec: AVPacket (压缩数据)
    Dec->>Filter: AVFrame (YUV/PCM)
    Filter->>Filter: 处理 (缩放/旋转等)
    Filter->>Enc: AVFrame (处理后)
    Enc->>Mux: AVPacket (重新编码)
    Mux->>Out: 写入文件
```

---

## 5. 解码流程详解

### 5.1 解码流程图

```mermaid
flowchart TD
    A[InputFile::open] --> B[avformat_open_input]
    B --> C{成功?}
    C -->|否| Z[返回错误]
    C -->|是| D[avformat_find_stream_info]
    D --> E[add_input_streams]
    E --> F[为每个流创建 InputStream]
    F --> G[choose_decoder 选择解码器]
    G --> H[avcodec_alloc_context3]
    H --> I[avcodec_parameters_to_context]
    I --> J[avcodec_open2 打开解码器]
    J --> K[init_filters 初始化过滤器]
    K --> L[返回成功]

    subgraph "读取帧循环"
        M[read_frames] --> N[av_read_frame]
        N --> O{返回值?}
        O -->|EAGAIN| N
        O -->|EOF| P[设置 eof_reached]
        O -->|成功| Q[process_input_packet]
        Q --> R{流类型?}
        R -->|Video| S[decode_video]
        R -->|Audio| T[decode_audio]
        S --> U[avcodec_send_packet]
        T --> U
        U --> V[avcodec_receive_frame]
        V --> W{got_output?}
        W -->|是| X[send_frame_to_filters]
        X --> Y[stream_copy_frame 保存帧]
        W -->|否| N
    end
```

### 5.2 解码核心代码流程

```cpp
// 1. 打开输入文件
int InputFile::open(const std::string &filename, FormatContext &format_ctx) {
    // 分配 AVFormatContext
    AVFormatContext *ifmt_ctx = avformat_alloc_context();
    
    // 打开输入
    ret = avformat_open_input(&ifmt_ctx, filename.c_str(), ...);
    
    // 查找流信息
    ret = avformat_find_stream_info(ifmt_ctx_.get(), opts);
    
    // 添加输入流
    ret = add_input_streams();
    
    // 初始化过滤器
    ret = init_filters();
}

// 2. 读取和解码帧
int InputFile::read_frames(...) {
    while (!stop_waiting()) {
        // 读取压缩包
        ret = av_read_frame(ifmt_ctx_.get(), pkt_);
        
        // 处理数据包
        ret = process_input_packet(ist, pkt, true);
    }
}

// 3. 解码视频
int InputFile::decode_video(...) {
    // 发送数据包到解码器
    ret = decode(ist->codec_ctx_.get(), pkt, decoded_frame.get(), got_output);
    
    if (got_output) {
        // 设置时间基
        decoded_frame->time_base = st->time_base;
        
        // 发送到过滤器
        ret = send_frame_to_filters(ist, decoded_frame);
    }
}
```

---

## 6. 编码流程详解

### 6.1 编码流程图

```mermaid
flowchart TD
    A[OutputFile::open] --> B[avformat_alloc_output_context2]
    B --> C{成功?}
    C -->|否| Z[返回错误]
    C -->|是| D[create_streams]
    D --> E[new_output_stream 创建输出流]
    E --> F[choose_encoder 选择编码器]
    F --> G[avcodec_alloc_context3]
    G --> H[init_filters]
    H --> I[avio_open 打开输出]
    I --> J[返回成功]

    subgraph "写入帧循环"
        K[write_frames] --> L[write_frame]
        L --> M{需要初始化?}
        M -->|是| N[init_output_stream_wrapper]
        N --> O[init_output_stream_encode]
        O --> P[avcodec_open2 打开编码器]
        P --> Q[of_check_init]
        Q --> R[avformat_write_header]
        M -->|否| S[of_encode_frame]
        R --> S
        S --> T[avcodec_send_frame]
        T --> U[avcodec_receive_packet]
        U --> V{有输出?}
        V -->|是| W[of_write_packet]
        W --> X[av_interleaved_write_frame]
        V -->|否| K
    end

    subgraph "结束处理"
        Y[flush] --> Y1[flush_one_encoder]
        Y1 --> Y2[of_encode_frame NULL]
        Y2 --> Y3[of_write_trailer]
        Y3 --> Y4[av_write_trailer]
    end
```

### 6.2 编码核心代码流程

```cpp
// 1. 打开输出文件
int OutputFile::open(const std::string &filename, FormatContext &format_ctx) {
    // 分配输出格式上下文
    avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, filename.c_str());
    
    // 创建输出流
    ret = create_streams(format_ctx);
    
    // 初始化过滤器
    ret = init_filters();
    
    // 打开输出 IO
    ret = avio_open(&ofmt_ctx_->pb, filename.c_str(), AVIO_FLAG_WRITE);
}

// 2. 写入帧
int OutputFile::write_frame(const Frame &raw_frame) {
    // 初始化输出流（首次）
    ret = init_output_stream_wrapper(ost, frame.get());
    
    // 编码并写入
    ret = of_encode_frame(ost, frame);
}

// 3. 编码帧
int OutputFile::of_encode_frame(...) {
    // 发送帧到编码器
    ret = avcodec_send_frame(enc_ctx, frame.get());
    
    while (1) {
        // 接收编码后的数据包
        ret = avcodec_receive_packet(enc_ctx, pkt.get());
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        
        // 写入数据包
        ret = of_write_packet(ost, pkt.get());
    }
}

// 4. 刷新并关闭
int OutputFile::flush() {
    // 刷新编码器
    for (auto &ost : output_streams_) {
        ret = flush_one_encoder(i);
    }
    
    // 写入尾部
    ret = of_write_trailer();
}
```

---

## 7. Filter 过滤器系统

### 7.1 过滤器架构

```mermaid
flowchart LR
    subgraph "BufferSrc"
        A[AVFrame 输入]
    end

    subgraph "Filter Chain"
        B[scale<br/>缩放]
        C[transpose<br/>旋转]
        D[fps<br/>帧率调整]
    end

    subgraph "BufferSink"
        E[AVFrame 输出]
    end

    A --> B --> C --> D --> E
```

### 7.2 过滤器初始化流程

```mermaid
sequenceDiagram
    participant FG as FilterGraph
    participant IF as InputFilter
    participant OF as OutputFilter
    participant FFmpeg as AVFilterGraph

    FG->>FG: init_simple_filtergraph()
    FG->>IF: 创建 InputFilter
    FG->>OF: 创建 OutputFilter
    FG->>FFmpeg: avfilter_graph_alloc()
    FG->>FFmpeg: avfilter_graph_parse2(graph_desc)
    IF->>FFmpeg: configure_input_filter()
    IF->>FFmpeg: 创建 buffersrc
    OF->>FFmpeg: configure_output_filter()
    OF->>FFmpeg: 创建 buffersink
    FG->>FFmpeg: avfilter_graph_config()
```

### 7.3 常用过滤器

| 过滤器 | 类型 | 说明 | 示例 |
|--------|------|------|------|
| `null` | 视频 | 视频直通 | `null` |
| `anull` | 音频 | 音频直通 | `anull` |
| `scale` | 视频 | 缩放 | `scale=1280:720` |
| `transpose` | 视频 | 旋转 | `transpose=1` (顺时针90°) |
| `fps` | 视频 | 帧率调整 | `fps=30` |
| `framestep` | 视频 | 跳帧 | `framestep=2` |
| `crop` | 视频 | 裁剪 | `crop=640:480:0:0` |
| `aresample` | 音频 | 重采样 | `aresample=44100` |
| `volume` | 音频 | 音量调节 | `volume=0.5` |

---

## 8. 关键数据结构

### 8.1 Frame 结构

```cpp
struct Frame {
    std::shared_ptr<AVPacket> packet;  // 压缩数据（流复制模式）
    std::shared_ptr<AVFrame> frame;    // 原始数据（解码模式）
    
    int64_t frame_number = 0;          // 帧序号
    AVRational time_base;              // 时间基
    int64_t pts = AV_NOPTS_VALUE;      // 显示时间戳
    AVMediaType codec_type;            // 媒体类型 (VIDEO/AUDIO)
    AVCodecID codec_id;                // 编解码器 ID
};
```

### 8.2 FormatContext 结构

```cpp
struct FormatContext {
    std::shared_ptr<AVFormatContext> av_format_context;  // FFmpeg 格式上下文
    AVStream* video_stream = nullptr;                     // 视频流
    std::shared_ptr<AVCodecContext> video_codec_context;  // 视频编解码上下文
    AVStream* audio_stream = nullptr;                     // 音频流
    std::shared_ptr<AVCodecContext> audio_codec_context;  // 音频编解码上下文
};
```

### 8.3 数据结构关系图

```mermaid
erDiagram
    InputFile ||--o{ InputStream : contains
    InputFile ||--|| AVFormatContext : has
    InputStream ||--|| AVCodecContext : has
    InputStream ||--|| FilterGraph : uses
    InputStream ||--o{ Frame : produces

    OutputFile ||--o{ OutputStream : contains
    OutputFile ||--|| AVFormatContext : has
    OutputStream ||--|| AVCodecContext : has
    OutputStream ||--|| FilterGraph : uses
    OutputStream ||--o{ Frame : consumes

    FilterGraph ||--o{ InputFilter : has
    FilterGraph ||--o{ OutputFilter : has
    FilterGraph ||--|| AVFilterGraph : wraps

    Frame ||--o| AVPacket : contains
    Frame ||--o| AVFrame : contains
```

---

## 9. 使用示例

### 9.1 基本转码流程

```cpp
#include "cv/video/input_file.h"
#include "cv/video/output_file.h"

int transcode(const std::string &input_path, const std::string &output_path) {
    kingfisher::cv::InputFile input_file;
    kingfisher::cv::OutputFile output_file;
    kingfisher::cv::FormatContext format_ctx;

    // 1. 打开输入文件
    int ret = input_file.open(input_path, format_ctx);
    if (ret < 0) return ret;

    // 2. 打开输出文件
    ret = output_file.open(output_path, format_ctx);
    if (ret < 0) return ret;

    // 3. 读取并写入帧
    bool finished = false;
    while (!finished) {
        std::vector<kingfisher::cv::Frame> video_frames, audio_frames;
        
        ret = input_file.read_frames(video_frames, audio_frames, 10, finished);
        if (ret < 0) break;

        if (!video_frames.empty()) {
            ret = output_file.write_frames(video_frames);
            if (ret < 0) break;
        }
        if (!audio_frames.empty()) {
            ret = output_file.write_frames(audio_frames);
            if (ret < 0) break;
        }
    }

    // 4. 刷新输出
    output_file.flush();

    return ret;
}
```

### 9.2 使用自定义过滤器

```cpp
kingfisher::cv::InputFile input_file;

// 设置视频过滤器：缩放到 1280x720
input_file.video_filter_spec_ = "scale=1280:720";

// 设置音频过滤器：重采样到 44100Hz
input_file.audio_filter_spec_ = "aresample=44100";

// 组合多个过滤器
input_file.video_filter_spec_ = "scale=1280:720,transpose=1,fps=30";

// 打开文件
input_file.open(input_path, format_ctx);
```

### 9.3 流程时序图

```mermaid
sequenceDiagram
    participant App as 应用程序
    participant IF as InputFile
    participant OF as OutputFile
    participant FFmpeg as FFmpeg

    App->>IF: open(input_path)
    IF->>FFmpeg: avformat_open_input()
    IF->>FFmpeg: avformat_find_stream_info()
    IF->>FFmpeg: avcodec_open2() [解码器]
    IF-->>App: FormatContext

    App->>OF: open(output_path, format_ctx)
    OF->>FFmpeg: avformat_alloc_output_context2()
    OF->>FFmpeg: avformat_new_stream()
    OF->>FFmpeg: avio_open()
    OF-->>App: success

    loop 每批次帧
        App->>IF: read_frames(batch_size)
        IF->>FFmpeg: av_read_frame()
        IF->>FFmpeg: avcodec_send_packet()
        IF->>FFmpeg: avcodec_receive_frame()
        IF-->>App: video_frames, audio_frames

        App->>OF: write_frames(frames)
        OF->>FFmpeg: avcodec_send_frame()
        OF->>FFmpeg: avcodec_receive_packet()
        OF->>FFmpeg: av_interleaved_write_frame()
    end

    App->>OF: flush()
    OF->>FFmpeg: avcodec_send_frame(NULL)
    OF->>FFmpeg: av_write_trailer()
```

---

## 附录：状态机

### 解码器状态

```mermaid
stateDiagram-v2
    [*] --> Uninitialized
    Uninitialized --> Opened: avcodec_open2()
    Opened --> Decoding: avcodec_send_packet()
    Decoding --> Decoding: avcodec_receive_frame()
    Decoding --> Draining: send NULL packet
    Draining --> Draining: avcodec_receive_frame()
    Draining --> Flushed: AVERROR_EOF
    Flushed --> [*]: avcodec_close()
```

### 编码器状态

```mermaid
stateDiagram-v2
    [*] --> Uninitialized
    Uninitialized --> Opened: avcodec_open2()
    Opened --> Encoding: avcodec_send_frame()
    Encoding --> Encoding: avcodec_receive_packet()
    Encoding --> Draining: send NULL frame
    Draining --> Draining: avcodec_receive_packet()
    Draining --> Flushed: AVERROR_EOF
    Flushed --> [*]: avcodec_close()
```

---

## 总结

Kingfisher 视频处理模块基于 FFmpeg 构建，提供了以下核心能力：

| 组件 | 职责 | 关键方法 |
|------|------|----------|
| `InputFile` | 输入文件管理、解码 | `open()`, `read_frames()` |
| `OutputFile` | 输出文件管理、编码 | `open()`, `write_frames()`, `flush()` |
| `InputStream` | 输入流状态管理 | `init_input_stream()` |
| `OutputStream` | 输出流状态管理 | 编码参数配置 |
| `FilterGraph` | 过滤器链管理 | `configure_filtergraph()`, `reap_filters()` |
| `Frame` | 统一帧数据封装 | 支持 AVPacket/AVFrame |

模块设计遵循 FFmpeg 官方 `ffmpeg.c` 的处理模式，同时进行了 C++ 封装和内存安全优化。
