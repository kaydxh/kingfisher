#ifndef KINGFISHER_PKG_CV_VIDEO_OUTPUT_STREAM_H_
#define KINGFISHER_PKG_CV_VIDEO_OUTPUT_STREAM_H_

#include "cv/video/input_filter.h"
#include "cv/video/stream.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/bsf.h"
#include "libavformat/avformat.h"
#include "libavutil/eval.h"
#include "libavutil/fifo.h"
};

namespace kingfisher {
namespace cv {

class OutputFilter;

enum OSTFinished {
  ENCODER_FINISHED = 1,
  MUXER_FINISHED = 2,
};

enum forced_keyframes_const {
  FKF_N,
  FKF_N_FORCED,
  FKF_PREV_FORCED_N,
  FKF_PREV_FORCED_T,
  FKF_T,
  FKF_NB
};

class OutputStream : public Stream {
 public:
  OutputStream(std::shared_ptr<AVFormatContext> ifmt_ctx,
               std::weak_ptr<AVFormatContext> ofmt_ctx, int file_index,
               unsigned int stream_index);

  ~OutputStream();

  AVStream *input_av_stream() const;

 public:
  const AVClass *av_class_ = nullptr;
  std::shared_ptr<AVFormatContext> ifmt_ctx_;
  std::shared_ptr<FilterGraph> ofilt_;
  // int index;        /* stream index in the output file */
  int source_index_; /* InputStream index */
  // AVStream *st;                  /* stream in the output file */
  bool encoding_needed_ = false; /* true if encoding needed for this stream */
  int64_t frame_number_ = 0;
  /* input pts and corresponding output pts
     for A/V sync */
  struct InputStream *sync_ist; /* input stream to sync against */
  int64_t sync_opts;
  /* output frame counter, could be changed to some true timestamp */  // FIXME
                                                                       // look
                                                                       // at
                                                                       // frame_number
  /* pts of the first frame encoded for this stream, used for limiting
   * recording time */
  int64_t first_pts_ = AV_NOPTS_VALUE;
  /* dts of the last packet sent to the muxer */
  int64_t last_mux_dts_ = AV_NOPTS_VALUE;
  // the timebase of the packets sent to the muxer
  AVRational mux_timebase_;
  AVRational enc_timebase_;

  AVBSFContext *bsf_ctx;

  AVCodecContext *enc_ctx;
  AVCodecParameters *ref_par_; /* associated input codec parameters with
                                 encoders options applied */
  int64_t max_frames_ = 0;
  /*
  AVFrame *filtered_frame;
  AVFrame *last_frame;
  AVPacket *pkt;
  int64_t last_dropped;
  int64_t last_nb0_frames[3];
  */

  void *hwaccel_ctx_ = nullptr;

  /* video only */
  AVRational frame_rate;
  AVRational max_frame_rate_;
  // enum VideoSyncMethod vsync_method;
  bool is_cfr_ = false;
  const char *fps_mode_ = nullptr;
  int force_fps_ = 0;
  int top_field_first_ = 0;
  int rotate_overridden_ = 0;
  bool autoscale = true;
  int bits_per_raw_sample_ = 0;
  double rotate_override_value_ = 0;

  AVRational frame_aspect_ratio_;

  /* forced key frames */
  int64_t forced_kf_ref_pts_ = AV_NOPTS_VALUE;
  int64_t *forced_kf_pts_;
  int forced_kf_count;
  int forced_kf_index;
  char *forced_keyframes_ = nullptr;
  AVExpr *forced_keyframes_pexpr;
  double forced_keyframes_expr_const_values[FKF_NB];
  int dropped_keyframe_ = 0;

  /* audio only */
  int *audio_channels_map;   /* list of the channels id to pick from the source
                                stream */
  int audio_channels_mapped; /* number of channels in audio_channels_map */

  char *logfile_prefix;
  FILE *logfile;

  OutputFilter *filter_ = nullptr;
  char *avfilter_ = nullptr;
  char *filters_ = nullptr;  ///< filtergraph associated to the -filter option
  char *filters_script_ = nullptr;  ///< filtergraph script associated to the
                                    ///< -filter_script option

  AVDictionary *encoder_opts_ = nullptr;
  AVDictionary *sws_dict_ = nullptr;
  AVDictionary *swr_opts_ = nullptr;
  char *apad;
  // OSTFinished finished_ =
  //    false;       /* no more packets should be written for this stream */
  int unavailable; /* true if the steram is unavailable (possibly temporarily)
                    */
  int stream_copy_;

  // init_output_stream() has been called for this stream
  // The encoder and the bitstream filters have been initialized and the stream
  // parameters are set in the AVStream.
  int initialized_;

  int inputs_done;

  const char *attachment_filename;
  int streamcopy_started;
  int copy_initial_nonkeyframes;
  int copy_prior_start;
  char *disposition;

  int keep_pix_fmt;

  /* stats */
  // combined size of all the packets written
  uint64_t data_size_ = 0;
  // number of packets send to the muxer
  uint64_t packets_written_ = 0;
  // number of frames/samples sent to the encoder
  uint64_t frames_encoded;
  uint64_t samples_encoded;
  // number of packets received from the encoder
  uint64_t packets_encoded;

  /* packet quality factor */
  int quality;

  int max_muxing_queue_size;

  /* the packets are buffered here until the muxer is ready to be initialized */
  AVFifo *muxing_queue;

  /*
   * The size of the AVPackets' buffers in queue.
   * Updated when a packet is either pushed or pulled from the queue.
   */
  size_t muxing_queue_data_size;

  /* Threshold after which max_muxing_queue_size will be in effect */
  size_t muxing_queue_data_threshold;

  /* packet picture type */
  int pict_type;

  /* frame encode sum of squared error values */
  int64_t error[4];
};

}  // namespace cv
}  // namespace kingfisher

#endif
