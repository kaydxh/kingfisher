#ifndef KINGFISHER_PKG_CV_VIDEO_STREAM_H_
#define KINGFISHER_PKG_CV_VIDEO_STREAM_H_

#include <vector>

#include "ffmpeg_types.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
};
#include <memory>

namespace kingfisher {
namespace cv {

class Stream {
 public:
  Stream(std::weak_ptr<AVFormatContext> fmt_ctx, AVStream *st, int file_index,
         unsigned int stream_index);
  ~Stream();
  virtual AVStream *av_stream() const;

  std::string choose_pix_fmts(AVFilterGraph *graph) const;

 public:
  std::weak_ptr<AVFormatContext> fmt_ctx_;
  // AVFormatContext *fmt_ctx_;
  AVStream *st_;
  int file_index_ = 0;
  unsigned int stream_index_ = 0;
  std::shared_ptr<AVCodecContext> codec_ctx_;  // for decode or encode
  // std::shared_ptr<AVCodec> codec_;             // for decode or encode
  std::shared_ptr<AVFrame> frame_;
  std::shared_ptr<AVPacket> pkt_;

  AVDictionary *codec_opts_ = nullptr;
  AVDictionary *sws_dict_ = nullptr;
  AVDictionary *swr_opts_ = nullptr;
  bool reinit_filters_ = false;

  AVRational framerate_; /* framerate forced with -r */
  int top_field_first_ = -1;
  int autorotate_ = 1;
  bool autoscale_ = true;

  int64_t frame_number_ = 0;
  std::vector<Frame> frames_;
  std::vector<Frame> video_frames_;
  std::vector<Frame> audio_frames_;
  bool keep_pix_fmt_ = false;
};

}  // namespace cv
}  // namespace kingfisher

#endif
