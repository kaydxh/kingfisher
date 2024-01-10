#include "input_file.h"

#include "ffmpeg_error.h"

namespace kingfisher {
namespace cv {

InputFile::InputFile() {}

InputFile::~InputFile() {}

int InputFile::open(const std::string &filename, AVFormatContext &format_ctx) {
  AVFormatContext *ifmt_ctx = nullptr;
  const AVInputFormat *file_iformat = nullptr;
  int ret = 0;
  if (format_.empty()) {
    if (!(file_iformat = av_find_input_format(format_.c_str()))) {
      av_log(this, AV_LOG_FATAL, "Unknown input format: '%s'\n",
             format_.c_str());
      ret = AVERROR_UNKNOWN;
      return ret;
    }
  }

  ret = avformat_find_stream_info(ifmt_ctx, nullptr);
  if (ret < 0) {
    av_log(nullptr, AV_LOG_ERROR, "Cannot find stream information\n");
    return ret;
  }

  /* get default parameters from command line */
  ifmt_ctx = avformat_alloc_context();
  if (!ifmt_ctx) {
    av_log(this, AV_LOG_ERROR, "failed to alloc avformat context -- %s\n",
           av_err2str(AVERROR(ENOMEM)));
    return AVERROR(ENOMEM);
  }

  ifmt_ctx->flags |= AVFMT_FLAG_NONBLOCK;
  /*
  if (bitexact_) {
    ifmt_ctx->flags |= AVFMT_FLAG_BITEXACT;
  }
  */

  if (!av_dict_get(format_opts_, "scan_all_pmts", nullptr,
                   AV_DICT_MATCH_CASE)) {
    av_dict_set(&format_opts_, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
  }

  /* open the input file with generic avformat function */
  ret = avformat_open_input(&ifmt_ctx, filename.c_str(), file_iformat,
                            &format_opts_);
  if (ret < 0) {
    if (ret == AVERROR_PROTOCOL_NOT_FOUND) {
      av_log(nullptr, AV_LOG_ERROR, "Did you mean file:%s?\n",
             filename.c_str());
      return ret;
    }
  }

  return 0;
}

}  // namespace cv
}  // namespace kingfisher
