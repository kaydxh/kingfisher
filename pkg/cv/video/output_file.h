#ifndef KINGFISHER_PKG_CV_VIDEO_OUTPUT_FILE_H_
#define KINGFISHER_PKG_CV_VIDEO_OUTPUT_FILE_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "ffmpeg_types.h"

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

  int open(const std::string &filename, FormatContext &format_ctx);
  int write_frames(const std::vector<Frame> &raw_frames);
  int flush();

  // 设置进度回调
  // callback: 进度回调函数，每写入一批帧后调用
  // interval: 回调间隔（帧数），默认每 10 帧回调一次
  void set_progress_callback(ProgressCallback callback, int interval = 10);

  // 设置取消回调
  // callback: 取消检查函数，返回 true 时停止写入
  void set_cancel_callback(CancelCallback callback);

  // 检查是否已取消
  bool is_cancelled() const;

  // 设置总帧数（用于进度计算）
  void set_total_frames(int64_t total_frames);

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
  int of_write_trailer();
  int of_encode_frame(const std::shared_ptr<OutputStream> &ost,
                      const std::shared_ptr<AVFrame> &frame);
  int flush_one_encoder(unsigned int stream_index);

  // int create_streams(const std::shared_ptr<AVFormatContext> &format_ctx);
  // int create_streams(const AVFormatContext &format_ctx);
  int create_streams(const FormatContext &format_ctx);

  int find_encoder(const std::string &name, enum AVMediaType type,
                   const AVCodec *&codec) const;

  int choose_encoder(const std::shared_ptr<OutputStream> &ost,
                     const AVCodec *&codec);

  // int new_output_stream(const std::shared_ptr<AVFormatContext> &ifmt_ctx,
  //                      enum AVMediaType type);
  //  int new_output_stream(const FormatContext &ifmt_ctx,
  //                       unsigned int stream_index, enum AVMediaType type);

  int new_output_stream(
      const std::shared_ptr<AVFormatContext> ifmt_ctx,
      const std::shared_ptr<AVCodecContext> input_codec_context,
      unsigned int stream_index, enum AVMediaType type);

  int init_filter(std::shared_ptr<OutputStream> &ost);
  int init_filters();

  int write_packet(const std::shared_ptr<AVPacket> &enc_pkt,
                   std::shared_ptr<OutputStream> &ost);

  int write_frame(int stream_index, const Frame &frame);
  int write_frame(const Frame &raw_frame);
  int write_avframe(int stream_index, const std::shared_ptr<AVFrame> &frame);

 public:
  const AVClass *av_class_ = nullptr;

  AVDictionary *command_opts_ = nullptr;
  AVDictionary *encoder_opts_ = nullptr;

  int file_index_ = 0;
  std::shared_ptr<AVFormatContext> ofmt_ctx_;

  const AVOutputFormat *format_ = nullptr;
  int recast_media_ = 0;

  AVDictionary *opts_ = nullptr;
  int ost_index_ = 0; /* index of the first stream in output_streams */
  int64_t recording_time_ =
      INT64_MAX;  ///< desired length of the resulting file in
                  ///< microseconds == AV_TIME_BASE units
  int64_t start_time_ =
      AV_NOPTS_VALUE;  ///< start time in microseconds == AV_TIME_BASE units
  uint64_t limit_filesize_ = UINT64_MAX; /* filesize limit expressed in bytes */

  int shortest_ = 0;

  bool header_written_ = false;
  bool bitexact_ = true;
  bool debug_ts_ = false;

  // GPU 硬件加速参数
  // gpu_id >= 0 时启用 NVENC 硬件编码（如 h264_nvenc, hevc_nvenc）
  // gpu_id < 0 时使用软件编码（默认）
  int64_t gpu_id_ = -1;

  // 当 GPU 编码器不可用时，是否自动切换到软件编码器
  bool auto_switch_to_soft_codec_ = true;

  // GOP 大小（关键帧间隔），仅对视频有效
  int gop_ = 250;

 private:
  std::vector<std::shared_ptr<OutputStream>> output_streams_;
  std::vector<std::shared_ptr<AVPacket>> muxing_queue_;
  int video_stream_index_ = -1;
  int audio_stream_index_ = -1;
  bool flush_once_ = false;  // 防止重复 flush

  // 进度回调相关
  ProgressCallback progress_callback_;
  int progress_callback_interval_ = 10;
  int64_t frames_written_ = 0;
  int64_t total_frames_ = 0;

  // 取消机制
  CancelCallback cancel_callback_;
  mutable bool cancelled_ = false;
};

}  // namespace cv
}  // namespace kingfisher

#endif
