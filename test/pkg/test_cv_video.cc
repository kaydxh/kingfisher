#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include "cv/video/ffmpeg_error.h"
#include "cv/video/ffmpeg_types.h"
#include "cv/video/input_file.h"
#include "cv/video/output_file.h"

using namespace kingfisher::cv;
/*
# 编译
make -j8

# 使用默认路径
./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 指定输入文件（输出自动生成为 video.copy.mp4）
VIDEO_INPUT=/path/to/video.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 同时指定输入和输出
VIDEO_INPUT=/path/to/input.mp4 VIDEO_OUTPUT=/path/to/output.mp4 ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 使用视频过滤器（缩放到 1280x720）
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="scale=1280:720" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 使用视频过滤器（旋转 90 度）
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="transpose=1" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 使用多个视频过滤器组合
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="scale=1280:720,transpose=1" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 使用音频过滤器（重采样到 44100Hz）
VIDEO_INPUT=/path/to/input.mp4 AUDIO_FILTER="aresample=44100" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 同时使用视频和音频过滤器
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="scale=1280:720" AUDIO_FILTER="volume=0.5" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 跳帧 - 每 2 帧取 1 帧
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="framestep=2" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 跳帧 - 每 5 帧取 1 帧
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="framestep=5" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 跳帧 - 降低到指定帧率（如 10fps）
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="fps=10" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

# 跳帧 + 缩放组合
VIDEO_INPUT=/path/to/input.mp4 VIDEO_FILTER="framestep=2,scale=1280:720" ./output/bin/kingfisher_base_test --gtest_filter=test_Video.*

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

  av_log(nullptr, AV_LOG_INFO, "Input: %s\n", input_url.c_str());
  av_log(nullptr, AV_LOG_INFO, "Output: %s\n", output_url.c_str());
  if (!video_filter.empty()) {
    av_log(nullptr, AV_LOG_INFO, "Video Filter: %s\n", video_filter.c_str());
  }
  if (!audio_filter.empty()) {
    av_log(nullptr, AV_LOG_INFO, "Audio Filter: %s\n", audio_filter.c_str());
  }

  InputFile input_file;
  // 设置自定义过滤器
  input_file.video_filter_spec_ = video_filter;
  input_file.audio_filter_spec_ = audio_filter;

  // 输出文件使用默认编码器（重新编码）
  OutputFile output_file;
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
}


