#ifndef KINGFISHER_PKG_CV_VIDEO_INPUT_FILE_H_
#define KINGFISHER_PKG_CV_VIDEO_INPUT_FILE_H_

#include <cstdint>
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
  // av_class_ 必须是类的第一个成员，因为 FFmpeg 的 av_log 期望
  // 第一个参数指向一个以 AVClass* 开头的结构体
  const AVClass *av_class_ = nullptr;

 public:
  InputFile();
  ~InputFile();

  // int open(const std::string &filename, AVFormatContext &format_ctx);
  int open(const std::string &filename, FormatContext &format_ctx);

  int read_frames(std::vector<Frame> &video_frames,
                  std::vector<Frame> &audio_frames, int32_t batch_size,
                  bool &finished);

  // Seek 到指定时间位置
  // timestamp: 目标时间戳，单位为秒
  // flags: seek 标志，参考 AVSEEK_FLAG_*
  //   AVSEEK_FLAG_BACKWARD (1): 向后搜索到最近的关键帧
  //   AVSEEK_FLAG_BYTE (2): 按字节位置搜索
  //   AVSEEK_FLAG_ANY (4): 搜索到任意帧（不一定是关键帧）
  //   AVSEEK_FLAG_FRAME (8): 按帧号搜索
  // 返回值: 0 成功, < 0 失败
  int seek(double timestamp, int flags = AVSEEK_FLAG_BACKWARD);

  // Seek 到指定时间位置（使用 pts 时间戳）
  // pts: 目标 PTS 值
  // stream_index: 流索引，-1 表示使用默认流
  // flags: seek 标志
  int seek_pts(int64_t pts, int stream_index = -1,
               int flags = AVSEEK_FLAG_BACKWARD);

  // Seek 到指定帧号位置
  // frame_number: 目标帧号（从 0 开始）
  // 返回值: 0 成功, < 0 失败
  int seek_frame(int64_t frame_number);

  // 获取视频时长（秒）
  double get_duration() const;

  // 获取当前位置（秒）
  double get_position() const;

  // 获取总帧数（估算）
  int64_t get_total_frames() const;

  // 获取帧率
  double get_frame_rate() const;

  // 设置进度回调
  // callback: 进度回调函数，每处理一批帧后调用
  // interval: 回调间隔（帧数），默认每 10 帧回调一次
  void set_progress_callback(ProgressCallback callback, int interval = 10);

  // 设置取消回调
  // callback: 取消检查函数，返回 true 时停止读取
  void set_cancel_callback(CancelCallback callback);

  // 检查是否已取消
  bool is_cancelled() const;

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

  int read_batch_frames(std::vector<Frame> &video_frames_buffer,
                        std::vector<Frame> &audio_frames_buffer,
                        std::vector<Frame> &video_frames,
                        std::vector<Frame> &audio_frames, int32_t batch_size,
                        bool &finished);

  int read_to_frames(std::vector<Frame> &frames_buffer,
                     std::vector<Frame> &frames, int32_t batch_size);

  int stream_copy(const std::shared_ptr<InputStream> &ist, AVPacket *packet);
  int stream_copy_frame(const std::shared_ptr<InputStream> &ist,
                        AVFrame *frame);

  int init_filters();

  void get_format_context(FormatContext &format_ctx) const;

  // Seek 后清空缓冲区和重置状态
  void flush_after_seek();

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

  // 自定义过滤器参数
  // 视频过滤器，如 "scale=1280:720", "transpose=1", "scale=1280:720,transpose=1"
  std::string video_filter_spec_;
  // 音频过滤器，如 "aresample=44100", "volume=0.5"
  std::string audio_filter_spec_;

  // GPU 硬件加速参数
  // gpu_id >= 0 时启用 CUDA 硬件解码（如 h264_cuvid, hevc_cuvid）
  // gpu_id < 0 时使用软件解码（默认）
  int64_t gpu_id_ = -1;

  // 当 GPU 解码器不可用时，是否自动切换到软件解码器
  bool auto_switch_to_soft_codec_ = true;

 private:
  bool debug_ts_ = false;
  bool copy_ts_ = false;
  float dts_delta_threshold_ = 10;
  float dts_error_threshold_ = 3600 * 30;
  int first_video_stream_index_ = -1;
  int first_audio_stream_index_ = -1;

  // 进度回调相关
  ProgressCallback progress_callback_;
  int progress_callback_interval_ = 10;
  int64_t frames_since_last_callback_ = 0;

  // 取消机制
  CancelCallback cancel_callback_;
  mutable bool cancelled_ = false;
};

}  // namespace cv
}  // namespace kingfisher

#endif
