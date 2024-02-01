#ifndef KINGFISHER_PKG_CV_VIDEO_OUTPUT_FILE_H_
#define KINGFISHER_PKG_CV_VIDEO_OUTPUT_FILE_H_

#include <memory>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

namespace kingfisher {
namespace cv {

class OutputFile {
 public:
  OutputFile();
  ~OutputFile();

  int open(const std::string &filename, AVFormatContext &format_ctx);

 public:
  const AVClass *av_class_ = nullptr;
  int file_index_ = 0;
  std::shared_ptr<AVFormatContext> ofmt_ctx_;

  const AVOutputFormat *format;

  AVDictionary *opts_;
  int ost_index_ = 0; /* index of the first stream in output_streams */
  int64_t recording_time_ =
      INT64_MAX;  ///< desired length of the resulting file in
                  ///< microseconds == AV_TIME_BASE units
  int64_t start_time_ =
      AV_NOPTS_VALUE;  ///< start time in microseconds == AV_TIME_BASE units
  uint64_t limit_filesize_ = UINT64_MAX; /* filesize limit expressed in bytes */

  int shortest_ = 0;

  int header_written = 1;
};

}  // namespace cv
}  // namespace kingfisher

#endif
