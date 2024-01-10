#ifndef KINGFISHER_PKG_CV_VIDEO_INPUT_FILE_H_
#define KINGFISHER_PKG_CV_VIDEO_INPUT_FILE_H_

#include <memory>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

namespace kingfisher {
namespace cv {

class InputFile {
 public:
  InputFile();
  ~InputFile();

  int open(const std::string &filename, AVFormatContext &format_ctx);

 public:
  std::shared_ptr<AVFormatContext> *ctx_;
  bool eof_reached_ = false; /* true if eof reached */
  bool eagain_ = false;      /* true if last read attempt returned EAGAIN */
  int ist_index_ = 0;        /* index of first stream in input_streams */
  int loop_ = 0;         /* set number of times input stream should be looped */
  int64_t duration_ = 0; /* actual duration of the longest stream in a file
                       at the moment when looping happens */
  AVRational time_base_ = AVRational{1, 1}; /* time base of the duration */
  int64_t input_ts_offset_ = 0;
  int input_sync_ref_ = -1;

  int64_t ts_offset_ = 0;
  int64_t last_ts_ = 0;
  int64_t start_time_ = AV_NOPTS_VALUE; /* user-specified start time in
                         AV_TIME_BASE or AV_NOPTS_VALUE */
  int64_t recording_time = AV_NOPTS_VALUE;
  int nb_streams_ = 0;      /* number of stream that ffmpeg is aware of; may be
                          different      from ctx.nb_streams if new streams appear
                          during      av_read_frame() */
  int nb_streams_warn_ = 0; /* number of streams that the user was warned of */
  int rate_emu_ = 0;
  float readrate_ = 0;
  int accurate_seek = 1;

  AVPacket *pkt_ = nullptr;

  AVDictionary *format_opts_ = nullptr;
  std::string format_;
};

}  // namespace cv
}  // namespace kingfisher

#endif
