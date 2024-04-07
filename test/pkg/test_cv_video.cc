#include <gtest/gtest.h>

#include "cv/video/ffmpeg_error.h"
#include "cv/video/ffmpeg_types.h"
#include "cv/video/input_file.h"

using namespace kingfisher::cv;

class test_Video : public testing::Test {
 public:
  test_Video() {}
  ~test_Video() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

//./kingfisher_base_test --gtest_filter=test_Video.*
TEST_F(test_Video, Transcode) {
  std::string input_url = "./testdata/bodyhead.mp4";

  InputFile input_file;
  // av_dict_set(&input_file.command_opts_, "c:v", "copy", 0);
  //    av_dict_set(&input_file.command_opts_, "c:a", true ? "" : "copy", 0);
  //    av_dict_set_int(&input_file.decoder_opts_, "udu_sei", 1, 0);
  AVFormatContext format_ctx;
  int ret = input_file.open(input_url, format_ctx);
  if (ret != 0) {
    av_log(nullptr, AV_LOG_ERROR, "Error occurred: %s\n", av_err2str(ret));
    return;
  }

  bool finished = false;
  std::vector<Frame> video_frames;
  std::vector<Frame> audio_frames;

  while (!finished) {
    int ret = input_file.read_frames(video_frames, audio_frames, 8, finished);
    if (ret < 0) {
      av_log(nullptr, AV_LOG_ERROR, "failed to read_video_frames: %s\n",
             av_err2str(ret));
      return;
    }

    av_log(nullptr, AV_LOG_INFO, "read frame size %lu\n", video_frames.size());

    for (unsigned int i = 0; i < video_frames.size(); ++i) {
      av_log(nullptr, AV_LOG_INFO,
             "read video frame number %ld,packet size: %d, codec_id: %d, "
             "pts: %ld \n",
             video_frames[i].frame_number, video_frames[i].frame->width,
             video_frames[i].codec_id, video_frames[i].pts);
    }

    for (unsigned int i = 0; i < audio_frames.size(); ++i) {
      av_log(nullptr, AV_LOG_INFO,
             "read audio frame number %ld,packet size: %d, codec_id: %d, "
             "pts: %ld \n",
             audio_frames[i].frame_number, audio_frames[i].frame->pkt_size,
             audio_frames[i].codec_type, audio_frames[i].pts);
    }
  }

  while (1) {
    sleep(1);
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
}
