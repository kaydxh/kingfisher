#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>

#include "cv/video/ffmpeg_error.h"
#include "cv/video/ffmpeg_types.h"
#include "cv/video/input_file.h"
#include "cv/video/output_file.h"

using namespace kingfisher::cv;
/*
# 编译
make -j8

# 使用默认路径
./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 指定输入文件（输出自动生成为 video.copy.mp4）
VIDEO_INPUT=/path/to/video.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 同时指定输入和输出
VIDEO_INPUT=/path/to/input.mp4 VIDEO_OUTPUT=/path/to/output.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 使用视频过滤器（缩放到 1280x720）
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="scale=1280:720" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 使用视频过滤器（旋转 90 度）
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="transpose=1" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 使用多个视频过滤器组合
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="scale=1280:720,transpose=1" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 使用音频过滤器（重采样到 44100Hz）
VIDEO_INPUT=/path/to/input.mp4 AUDIO_FILTER="aresample=44100" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 同时使用视频和音频过滤器
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="scale=1280:720" AUDIO_FILTER="volume=0.5" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 跳帧 - 每 2 帧取 1 帧
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="framestep=2" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 跳帧 - 每 5 帧取 1 帧
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="framestep=5" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 跳帧 - 降低到指定帧率（如 10fps）
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="fps=10" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 跳帧 + 缩放组合
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="framestep=2,scale=1280:720" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# 使用 GPU 硬件加速（CUDA 解码 + NVENC 编码）
GPU_ID=0 VIDEO_INPUT=/path/to/input.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# GPU 加速 + 视频滤镜
GPU_ID=0 VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="scale=1280:720" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Transcode

# Seek 测试（默认 seek 到视频中间位置）
VIDEO_INPUT=/path/to/input.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Seek

# Seek 测试 - 指定 seek 时间（秒）
SEEK_TIME=10.5 VIDEO_INPUT=/path/to/input.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Seek

# Seek 测试 - 指定 seek 帧号
SEEK_FRAME=250 VIDEO_INPUT=/path/to/input.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Seek

# Seek 测试 + GPU 加速
GPU_ID=0 SEEK_TIME=5.0 VIDEO_INPUT=/path/to/input.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Seek

# 进度回调测试
VIDEO_INPUT=/path/to/input.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.ProgressCallback

# 取消机制测试（在处理 50% 后取消）
CANCEL_AT=0.5 VIDEO_INPUT=/path/to/input.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.CancelCallback

# 带进度回调的完整转码测试
VIDEO_INPUT=/path/to/input.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.TranscodeWithProgress

# 获取视频帧数
ffprobe -v error -select_streams v:0 -count_packets -show_entries stream=nb_read_packets -of csv=p=0 input.mp4
ffprobe -v error -select_streams v:0 -count_frames -show_entries stream=nb_read_frames -of csv=p=0 input.mp4

*/

// 生成默认输出路径：在文件名后添加 .copy
static std::string generate_output_url(const std::string& input_url) {
  size_t dot_pos = input_url.rfind('.');
  if (dot_pos != std::string::npos) {
    return input_url.substr(0, dot_pos) + ".copy" + input_url.substr(dot_pos);
  }
  return input_url + ".copy";
}

// 从环境变量获取参数
static std::string get_input_url() {
  const char* env = std::getenv("VIDEO_INPUT");
  return env ? env : "./testdata/sce_video.mp4";
}

static std::string get_output_url(const std::string& input_url) {
  const char* env = std::getenv("VIDEO_OUTPUT");
  return env ? env : generate_output_url(input_url);
}

// 获取视频过滤器
static std::string get_video_filter() {
  const char* env = std::getenv("VIDEO_FILTER");
  return env ? env : "";
}

// 获取音频过滤器
static std::string get_audio_filter() {
  const char* env = std::getenv("AUDIO_FILTER");
  return env ? env : "";
}

// 获取 GPU ID（-1 表示使用软件编解码，>= 0 表示使用指定 GPU）
static int64_t get_gpu_id() {
  const char* env = std::getenv("GPU_ID");
  return env ? std::stoll(env) : -1;
}

class test_Video : public testing::Test {
 public:
  test_Video() {}
  ~test_Video() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

// ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*
// 或使用环境变量: VIDEO_INPUT=/path/to/input.mp4 VIDEO_OUTPUT=/path/to/output.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*
TEST_F(test_Video, Transcode) {
  std::string input_url = get_input_url();
  std::string output_url = get_output_url(input_url);
  std::string video_filter = get_video_filter();
  std::string audio_filter = get_audio_filter();
  int64_t gpu_id = get_gpu_id();

  av_log(nullptr, AV_LOG_INFO, "Input: %s\n", input_url.c_str());
  av_log(nullptr, AV_LOG_INFO, "Output: %s\n", output_url.c_str());
  if (!video_filter.empty()) {
    av_log(nullptr, AV_LOG_INFO, "Video Filter: %s\n", video_filter.c_str());
  }
  if (!audio_filter.empty()) {
    av_log(nullptr, AV_LOG_INFO, "Audio Filter: %s\n", audio_filter.c_str());
  }
  if (gpu_id >= 0) {
    av_log(nullptr, AV_LOG_INFO, "GPU ID: %" PRId64 " (Hardware Acceleration Enabled)\n", gpu_id);
  } else {
    av_log(nullptr, AV_LOG_INFO, "GPU ID: %" PRId64 " (Software Codec)\n", gpu_id);
  }

  InputFile input_file;
  // 设置自定义过滤器
  input_file.video_filter_spec_ = video_filter;
  input_file.audio_filter_spec_ = audio_filter;
  // 设置 GPU ID
  input_file.gpu_id_ = gpu_id;

  // 输出文件使用默认编码器（重新编码）
  OutputFile output_file;
  // 设置 GPU ID
  output_file.gpu_id_ = gpu_id;
  // AVFormatContext format_ctx;
  FormatContext format_ctx;
  int ret = input_file.open(input_url, format_ctx);
  if (ret != 0) {
    av_log(nullptr, AV_LOG_ERROR, "Error occurred: %s\n", av_err2str(ret));
    return;
  }

#if 1
  ret = output_file.open(output_url, format_ctx);
  if (ret != 0) {
    av_log(nullptr, AV_LOG_ERROR, "Error occurred: %s\n", av_err2str(ret));
    return;
  }
#endif

  bool finished = false;
  std::vector<Frame> video_frames;
  std::vector<Frame> audio_frames;

  while (!finished) {
    video_frames.clear();
    audio_frames.clear();

    int ret = input_file.read_frames(video_frames, audio_frames, 8, finished);
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR, "failed to read_video_frames: %s\n",
             av_err2str(ret));
      break;
    }

    av_log(nullptr, AV_LOG_INFO, "read video frame size %lu, audio frame size %lu\n",
           video_frames.size(), audio_frames.size());

    /*
    for (unsigned int i = 0; i < video_frames.size(); ++i) {

      av_log(nullptr, AV_LOG_INFO,
             "read video frame number %ld,packet size: %d, codec_id: %d, "
             "pts: %ld \n",
             video_frames[i].frame_number, video_frames[i].frame->width,
             video_frames[i].codec_id, video_frames[i].pts);
    }
    */

    for (unsigned int i = 0; i < audio_frames.size(); ++i) {
      av_log(nullptr, AV_LOG_INFO,
             "read audio frame number %ld,packet size: %d, codec_id: %d, "
             "pts: %ld \n",
             audio_frames[i].frame_number, audio_frames[i].frame->pkt_size,
             audio_frames[i].codec_type, audio_frames[i].pts);
    }

#if 1
    ret = output_file.write_frames(video_frames);
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR, "write_video_frames failed: %s\n",
             av_err2str(ret));
      break;
    }

    ret = output_file.write_frames(audio_frames);
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR, "write_audio_frames failed: %s\n",
             av_err2str(ret));
      break;
    }
#endif
  }

  // Flush encoder and write trailer
  ret = output_file.flush();
  if (ret < 0) {
    av_log(nullptr, AV_LOG_ERROR, "flush output_file failed: %s\n",
           av_err2str(ret));
  }

  av_log(nullptr, AV_LOG_INFO, "Transcode completed\n");
}

// Seek 功能测试
// SEEK_TIME=10.5 VIDEO_INPUT=/path/to/video.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Seek
// SEEK_FRAME=250 VIDEO_INPUT=/path/to/video.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.Seek
TEST_F(test_Video, Seek) {
  std::string input_url = get_input_url();
  int64_t gpu_id = get_gpu_id();

  av_log(nullptr, AV_LOG_INFO, "=== Seek Test ===\n");
  av_log(nullptr, AV_LOG_INFO, "Input: %s\n", input_url.c_str());

  InputFile input_file;
  input_file.gpu_id_ = gpu_id;

  FormatContext format_ctx;
  int ret = input_file.open(input_url, format_ctx);
  if (ret != 0) {
    av_log(nullptr, AV_LOG_ERROR, "Failed to open input file: %s\n", av_err2str(ret));
    FAIL();
    return;
  }

  // 获取视频基本信息
  double duration = input_file.get_duration();
  int64_t total_frames = input_file.get_total_frames();
  double fps = input_file.get_frame_rate();

  av_log(nullptr, AV_LOG_INFO, "Video Info:\n");
  av_log(nullptr, AV_LOG_INFO, "  Duration: %.2f seconds\n", duration);
  av_log(nullptr, AV_LOG_INFO, "  Total frames: %" PRId64 "\n", total_frames);
  av_log(nullptr, AV_LOG_INFO, "  Frame rate: %.2f fps\n", fps);

  EXPECT_GT(duration, 0.0);
  EXPECT_GT(fps, 0.0);

  // 读取开头几帧
  av_log(nullptr, AV_LOG_INFO, "\n--- Reading frames from beginning ---\n");
  std::vector<Frame> video_frames, audio_frames;
  bool finished = false;

  ret = input_file.read_frames(video_frames, audio_frames, 5, finished);
  EXPECT_GE(ret, 0);
  av_log(nullptr, AV_LOG_INFO, "Read %lu video frames, %lu audio frames from beginning\n",
         video_frames.size(), audio_frames.size());

  if (!video_frames.empty()) {
    av_log(nullptr, AV_LOG_INFO, "First frame PTS: %" PRId64 "\n", video_frames[0].pts);
  }

  // 测试 seek 到中间位置（按时间）
  double seek_time = duration / 2.0;
  const char* env_seek_time = std::getenv("SEEK_TIME");
  if (env_seek_time) {
    seek_time = std::stod(env_seek_time);
  }

  av_log(nullptr, AV_LOG_INFO, "\n--- Seeking to %.2f seconds ---\n", seek_time);
  ret = input_file.seek(seek_time);
  EXPECT_EQ(ret, 0);

  if (ret == 0) {
    video_frames.clear();
    audio_frames.clear();
    finished = false;

    ret = input_file.read_frames(video_frames, audio_frames, 5, finished);
    EXPECT_GE(ret, 0);
    av_log(nullptr, AV_LOG_INFO, "Read %lu video frames, %lu audio frames after seek to %.2f s\n",
           video_frames.size(), audio_frames.size(), seek_time);

    if (!video_frames.empty()) {
      av_log(nullptr, AV_LOG_INFO, "Frame after seek - PTS: %" PRId64 ", frame_number: %" PRId64 "\n",
             video_frames[0].pts, video_frames[0].frame_number);
    }
  }

  // 测试 seek_frame（按帧号）
  int64_t seek_frame_num = total_frames / 4;
  const char* env_seek_frame = std::getenv("SEEK_FRAME");
  if (env_seek_frame) {
    seek_frame_num = std::stoll(env_seek_frame);
  }

  if (seek_frame_num > 0 && fps > 0) {
    av_log(nullptr, AV_LOG_INFO, "\n--- Seeking to frame %" PRId64 " ---\n", seek_frame_num);
    ret = input_file.seek_frame(seek_frame_num);
    EXPECT_EQ(ret, 0);

    if (ret == 0) {
      video_frames.clear();
      audio_frames.clear();
      finished = false;

      ret = input_file.read_frames(video_frames, audio_frames, 5, finished);
      EXPECT_GE(ret, 0);
      av_log(nullptr, AV_LOG_INFO, "Read %lu video frames, %lu audio frames after seek to frame %" PRId64 "\n",
             video_frames.size(), audio_frames.size(), seek_frame_num);

      if (!video_frames.empty()) {
        av_log(nullptr, AV_LOG_INFO, "Frame after seek_frame - PTS: %" PRId64 ", frame_number: %" PRId64 "\n",
               video_frames[0].pts, video_frames[0].frame_number);
      }
    }
  }

  // 测试 seek 回到开头
  av_log(nullptr, AV_LOG_INFO, "\n--- Seeking back to beginning (0.0 s) ---\n");
  ret = input_file.seek(0.0);
  EXPECT_EQ(ret, 0);

  if (ret == 0) {
    video_frames.clear();
    audio_frames.clear();
    finished = false;

    ret = input_file.read_frames(video_frames, audio_frames, 5, finished);
    EXPECT_GE(ret, 0);
    av_log(nullptr, AV_LOG_INFO, "Read %lu video frames, %lu audio frames after seek to beginning\n",
           video_frames.size(), audio_frames.size());

    if (!video_frames.empty()) {
      av_log(nullptr, AV_LOG_INFO, "First frame after seek back - PTS: %" PRId64 "\n", video_frames[0].pts);
    }
  }

  // 测试 seek 到接近结尾
  double near_end = duration - 1.0;
  if (near_end > 0) {
    av_log(nullptr, AV_LOG_INFO, "\n--- Seeking near end (%.2f s) ---\n", near_end);
    ret = input_file.seek(near_end);
    EXPECT_EQ(ret, 0);

    if (ret == 0) {
      video_frames.clear();
      audio_frames.clear();
      finished = false;

      ret = input_file.read_frames(video_frames, audio_frames, 10, finished);
      av_log(nullptr, AV_LOG_INFO, "Read %lu video frames near end, finished=%d\n",
             video_frames.size(), finished);
    }
  }

  av_log(nullptr, AV_LOG_INFO, "\n=== Seek Test Completed ===\n");
}

// 进度回调测试
// VIDEO_INPUT=/path/to/video.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.ProgressCallback
TEST_F(test_Video, ProgressCallback) {
  std::string input_url = get_input_url();
  int64_t gpu_id = get_gpu_id();

  av_log(nullptr, AV_LOG_INFO, "=== Progress Callback Test ===\n");
  av_log(nullptr, AV_LOG_INFO, "Input: %s\n", input_url.c_str());

  InputFile input_file;
  input_file.gpu_id_ = gpu_id;

  // 设置进度回调
  int callback_count = 0;
  input_file.set_progress_callback([&callback_count](const ProgressInfo &info) {
    callback_count++;
    av_log(nullptr, AV_LOG_INFO,
           "[Progress] %.1f%% - Frame: %" PRId64 "/%" PRId64 ", Time: %.2fs/%.2fs\n",
           info.progress * 100.0,
           info.current_frame, info.total_frames,
           info.current_seconds, info.total_seconds);
  }, 30);  // 每 30 帧回调一次

  FormatContext format_ctx;
  int ret = input_file.open(input_url, format_ctx);
  if (ret != 0) {
    av_log(nullptr, AV_LOG_ERROR, "Failed to open input file: %s\n", av_err2str(ret));
    FAIL();
    return;
  }

  av_log(nullptr, AV_LOG_INFO, "Duration: %.2f s, Total frames: %" PRId64 "\n",
         input_file.get_duration(), input_file.get_total_frames());

  std::vector<Frame> video_frames, audio_frames;
  bool finished = false;
  int64_t total_read_frames = 0;

  while (!finished) {
    video_frames.clear();
    audio_frames.clear();

    ret = input_file.read_frames(video_frames, audio_frames, 30, finished);
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR, "read_frames failed: %s\n", av_err2str(ret));
      break;
    }
    total_read_frames += video_frames.size();
  }

  av_log(nullptr, AV_LOG_INFO, "\n--- Results ---\n");
  av_log(nullptr, AV_LOG_INFO, "Total frames read: %" PRId64 "\n", total_read_frames);
  av_log(nullptr, AV_LOG_INFO, "Progress callbacks: %d\n", callback_count);

  EXPECT_GT(callback_count, 0);
  av_log(nullptr, AV_LOG_INFO, "\n=== Progress Callback Test Completed ===\n");
}

// 取消机制测试
// CANCEL_AT=0.5 VIDEO_INPUT=/path/to/video.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.CancelCallback
TEST_F(test_Video, CancelCallback) {
  std::string input_url = get_input_url();
  int64_t gpu_id = get_gpu_id();

  // 获取取消进度（默认 50%）
  double cancel_at = 0.5;
  const char* env_cancel_at = std::getenv("CANCEL_AT");
  if (env_cancel_at) {
    cancel_at = std::stod(env_cancel_at);
  }

  av_log(nullptr, AV_LOG_INFO, "=== Cancel Callback Test ===\n");
  av_log(nullptr, AV_LOG_INFO, "Input: %s\n", input_url.c_str());
  av_log(nullptr, AV_LOG_INFO, "Cancel at: %.1f%%\n", cancel_at * 100.0);

  InputFile input_file;
  input_file.gpu_id_ = gpu_id;

  // 用于取消的标志
  std::atomic<bool> should_cancel{false};
  double current_progress = 0.0;

  // 设置进度回调，更新当前进度
  input_file.set_progress_callback([&current_progress, &should_cancel, cancel_at](const ProgressInfo &info) {
    current_progress = info.progress;
    av_log(nullptr, AV_LOG_INFO,
           "[Progress] %.1f%% - Frame: %" PRId64 "\n",
           info.progress * 100.0, info.current_frame);
    
    // 达到取消进度时设置取消标志
    if (info.progress >= cancel_at && !should_cancel.load()) {
      av_log(nullptr, AV_LOG_WARNING, ">>> Triggering cancel at %.1f%% <<<\n", info.progress * 100.0);
      should_cancel.store(true);
    }
  }, 10);

  // 设置取消回调
  input_file.set_cancel_callback([&should_cancel]() {
    return should_cancel.load();
  });

  FormatContext format_ctx;
  int ret = input_file.open(input_url, format_ctx);
  if (ret != 0) {
    av_log(nullptr, AV_LOG_ERROR, "Failed to open input file: %s\n", av_err2str(ret));
    FAIL();
    return;
  }

  std::vector<Frame> video_frames, audio_frames;
  bool finished = false;
  int64_t total_read_frames = 0;
  bool was_cancelled = false;

  while (!finished) {
    video_frames.clear();
    audio_frames.clear();

    ret = input_file.read_frames(video_frames, audio_frames, 10, finished);
    if (ret == AVERROR_EXIT) {
      av_log(nullptr, AV_LOG_INFO, "Operation cancelled!\n");
      was_cancelled = true;
      break;
    }
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR, "read_frames failed: %s\n", av_err2str(ret));
      break;
    }
    total_read_frames += video_frames.size();
  }

  av_log(nullptr, AV_LOG_INFO, "\n--- Results ---\n");
  av_log(nullptr, AV_LOG_INFO, "Total frames read: %" PRId64 "\n", total_read_frames);
  av_log(nullptr, AV_LOG_INFO, "Final progress: %.1f%%\n", current_progress * 100.0);
  av_log(nullptr, AV_LOG_INFO, "Was cancelled: %s\n", was_cancelled ? "YES" : "NO");
  av_log(nullptr, AV_LOG_INFO, "is_cancelled(): %s\n", input_file.is_cancelled() ? "YES" : "NO");

  // 验证取消机制生效
  EXPECT_TRUE(was_cancelled);
  EXPECT_TRUE(input_file.is_cancelled());
  EXPECT_GE(current_progress, cancel_at - 0.1);  // 允许一定误差
  EXPECT_LT(current_progress, 1.0);  // 没有完成

  av_log(nullptr, AV_LOG_INFO, "\n=== Cancel Callback Test Completed ===\n");
}

// 带进度回调的完整转码测试
// VIDEO_INPUT=/path/to/video.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.TranscodeWithProgress
TEST_F(test_Video, TranscodeWithProgress) {
  std::string input_url = get_input_url();
  std::string output_url = get_output_url(input_url);
  int64_t gpu_id = get_gpu_id();

  av_log(nullptr, AV_LOG_INFO, "=== Transcode With Progress Test ===\n");
  av_log(nullptr, AV_LOG_INFO, "Input: %s\n", input_url.c_str());
  av_log(nullptr, AV_LOG_INFO, "Output: %s\n", output_url.c_str());

  InputFile input_file;
  input_file.gpu_id_ = gpu_id;

  OutputFile output_file;
  output_file.gpu_id_ = gpu_id;

  // 设置输入进度回调
  int read_callbacks = 0;
  input_file.set_progress_callback([&read_callbacks](const ProgressInfo &info) {
    read_callbacks++;
    av_log(nullptr, AV_LOG_INFO,
           "[READ ] %.1f%% - Frame: %" PRId64 ", Time: %.2fs\n",
           info.progress * 100.0, info.current_frame, info.current_seconds);
  }, 50);

  // 设置输出进度回调
  int write_callbacks = 0;
  output_file.set_progress_callback([&write_callbacks](const ProgressInfo &info) {
    write_callbacks++;
    av_log(nullptr, AV_LOG_INFO,
           "[WRITE] %.1f%% - Frame: %" PRId64 "\n",
           info.progress * 100.0, info.current_frame);
  }, 50);

  FormatContext format_ctx;
  int ret = input_file.open(input_url, format_ctx);
  if (ret != 0) {
    av_log(nullptr, AV_LOG_ERROR, "Failed to open input file: %s\n", av_err2str(ret));
    FAIL();
    return;
  }

  ret = output_file.open(output_url, format_ctx);
  if (ret != 0) {
    av_log(nullptr, AV_LOG_ERROR, "Failed to open output file: %s\n", av_err2str(ret));
    FAIL();
    return;
  }

  // 设置输出文件的总帧数，用于进度计算
  output_file.set_total_frames(input_file.get_total_frames());

  std::vector<Frame> video_frames, audio_frames;
  bool finished = false;
  auto start_time = std::chrono::steady_clock::now();

  while (!finished) {
    video_frames.clear();
    audio_frames.clear();

    ret = input_file.read_frames(video_frames, audio_frames, 30, finished);
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR, "read_frames failed: %s\n", av_err2str(ret));
      break;
    }

    ret = output_file.write_frames(video_frames);
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR, "write_video_frames failed: %s\n", av_err2str(ret));
      break;
    }

    ret = output_file.write_frames(audio_frames);
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR, "write_audio_frames failed: %s\n", av_err2str(ret));
      break;
    }
  }

  ret = output_file.flush();
  if (ret < 0) {
    av_log(nullptr, AV_LOG_ERROR, "flush failed: %s\n", av_err2str(ret));
  }

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  av_log(nullptr, AV_LOG_INFO, "\n--- Results ---\n");
  av_log(nullptr, AV_LOG_INFO, "Read progress callbacks: %d\n", read_callbacks);
  av_log(nullptr, AV_LOG_INFO, "Write progress callbacks: %d\n", write_callbacks);
  av_log(nullptr, AV_LOG_INFO, "Total time: %.2f seconds\n", duration.count() / 1000.0);

  EXPECT_GT(read_callbacks, 0);
  EXPECT_GT(write_callbacks, 0);

  av_log(nullptr, AV_LOG_INFO, "\n=== Transcode With Progress Test Completed ===\n");
}

#if 0
  // 打开输入文件
  AVFormatContext* format_ctx = NULL;
  if (avformat_open_input(&format_ctx, input_file.c_str(), NULL, NULL) != 0) {
    fprintf(stderr, "Could not open input file '%s'\n", input_file.c_str());
    return;
  }

  // 获取输入文件的流信息
  if (avformat_find_stream_info(format_ctx, NULL) < 0) {
    fprintf(stderr, "Could not find stream information\n");
    avformat_close_input(&format_ctx);
    return;
  }

  // 打印输入文件的详细信息
  av_dump_format(format_ctx, 0, input_file.c_str(), 0);

  // 读取 packet
  AVPacket packet;
  int read_frame_result;
  int count = 0;
  while ((read_frame_result = av_read_frame(format_ctx, &packet)) >= 0) {
    count++;
    printf("Read packet from stream %d, size %d, count: %d\n",
           packet.stream_index, packet.size, count);
    av_packet_unref(&packet);
  }

  if (read_frame_result != AVERROR_EOF) {
    fprintf(stderr, "Error reading packet\n");
  }

  // 关闭输入文件
  avformat_close_input(&format_ctx);
#endif
