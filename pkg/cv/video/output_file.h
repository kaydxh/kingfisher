#ifndef KINGFISHER_PKG_CV_VIDEO_OUTPUT_FILE_H_
#define KINGFISHER_PKG_CV_VIDEO_OUTPUT_FILE_H_

#include <memory>
#include <vector>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

namespace kingfisher {
namespace cv {

class OutputStream;

class OutputFile {
 public:
  OutputFile();
  ~OutputFile();

  int open(const std::string &filename, AVFormatContext &format_ctx);

 private:
  int init_output_stream_wrapper(const std::shared_ptr<OutputStream> &ost,
                                 AVFrame *frame);

  int init_output_stream_encode(const std::shared_ptr<OutputStream> &ost,
                                AVFrame *frame);

  int init_output_stream(const std::shared_ptr<OutputStream> &ost,
                         AVFrame *frame);

  void set_encoder_id(const std::shared_ptr<OutputStream> &ost);

  void init_encoder_time_base(const std::shared_ptr<OutputStream> &ost,
                              AVRational default_time_base);

  int init_output_stream_streamcopy(const std::shared_ptr<OutputStream> &ost);
  int of_check_init();
  int of_write_packet(const std::shared_ptr<OutputStream> &ost, AVPacket *pkt);

 public:
  const AVClass *av_class_ = nullptr;
  int file_index_ = 0;
  std::shared_ptr<AVFormatContext> ofmt_ctx_;

  const AVOutputFormat *format_;

  AVDictionary *opts_;
  int ost_index_ = 0; /* index of the first stream in output_streams */
  int64_t recording_time_ =
      INT64_MAX;  ///< desired length of the resulting file in
                  ///< microseconds == AV_TIME_BASE units
  int64_t start_time_ =
      AV_NOPTS_VALUE;  ///< start time in microseconds == AV_TIME_BASE units
  uint64_t limit_filesize_ = UINT64_MAX; /* filesize limit expressed in bytes */

  int shortest_ = 0;

  bool header_written_ = true;
  bool bitexact_ = true;
  bool debug_ts_ = false;

 private:
  std::vector<std::shared_ptr<OutputStream>> output_streams_;
  std::vector<std::shared_ptr<AVPacket>> muxing_queue_;
};

}  // namespace cv
}  // namespace kingfisher

#endif
