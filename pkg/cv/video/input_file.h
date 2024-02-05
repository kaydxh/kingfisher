#ifndef KINGFISHER_PKG_CV_VIDEO_INPUT_FILE_H_
#define KINGFISHER_PKG_CV_VIDEO_INPUT_FILE_H_

#include <functional>
#include <memory>
#include <vector>

#include "ffmpeg_types.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

namespace kingfisher {
namespace cv {

class InputStream;

class InputFile {
 public:
  InputFile();
  ~InputFile();

  int open(const std::string &filename, AVFormatContext &format_ctx);

  int read_video_frames(std::vector<Frame> &video_frames, int32_t batch_size,
                        bool &finished);

 private:
  int choose_decoder(const std::shared_ptr<InputStream> &ist,
                     const AVCodec *&codec);
  int add_input_streams();
  int find_decoder(const std::string &name, enum AVMediaType type,
                   const AVCodec *&codec) const;
  int process_input_packet(const std::shared_ptr<InputStream> &ist,
                           AVPacket *pkt, int no_eof);

  int decode_audio(const std::shared_ptr<InputStream> &ist, AVPacket *pkt,
                   bool &got_output, bool &decode_failed);

  int decode_video(const std::shared_ptr<InputStream> &ist, AVPacket *pkt,
                   int eof, bool &got_output, int64_t &duration_pts,
                   bool &decode_failed);

  int decode(AVCodecContext *avctx, AVPacket *pkt, AVFrame *frame,
             bool &got_frame);
  int check_decode_result(const std::shared_ptr<InputStream> &ist,
                          bool &got_output);

  int send_filter_eof(const std::shared_ptr<InputStream> &ist);
  int send_frame_to_filters(const std::shared_ptr<InputStream> &ist,
                            const std::shared_ptr<AVFrame> &decoded_frame);

  int read_frames(const std::function<bool()> &stop_waiting);

  int stream_copy(const std::shared_ptr<InputStream> &ist, AVPacket *packet);

 public:
  int file_index_ = 0;
  std::shared_ptr<AVFormatContext> ifmt_ctx_;
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
  int64_t start_time_eof_ = AV_NOPTS_VALUE;
  int seek_timestamp_ = 0;
  int64_t recording_time = AV_NOPTS_VALUE;
  int nb_streams_ = 0;      /* number of stream that ffmpeg is aware of; may be
                          different      from ctx.nb_streams if new streams appear
                          during      av_read_frame() */
  int nb_streams_warn_ = 0; /* number of streams that the user was warned of */
  int rate_emu_ = 0;
  float readrate_ = 0;
  int accurate_seek = 1;

  bool bitexact_ = true;

  AVPacket *pkt_ = nullptr;

  AVDictionary *format_opts_ = nullptr;
  AVDictionary *decoder_opts_ = nullptr;
  AVDictionary *command_opts_ = nullptr;
  std::string format_;
  bool find_stream_info_ = true;

  std::vector<std::shared_ptr<InputStream>> input_streams_;
  int recast_media_ = 0;

  bool video_disable_ = false;
  bool audio_disable_ = false;
  bool subtitle_disable_ = false;
  bool data_disable_ = false;

 private:
  const AVClass *av_class_ = nullptr;
  bool debug_ts_ = false;
  bool copy_ts_ = false;
  float dts_delta_threshold_ = 10;
  float dts_error_threshold_ = 3600 * 30;
  int first_video_stream_index_ = -1;
  int first_audio_stream_index_ = -1;
};

}  // namespace cv
}  // namespace kingfisher

#endif
