#ifndef KINGFISHER_PKG_CV_VIDEO_INPUT_STREAM_H_
#define KINGFISHER_PKG_CV_VIDEO_INPUT_STREAM_H_

#include <vector>

#include "cv/video/input_filter.h"
#include "stream.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

namespace kingfisher {
namespace cv {

enum HWAccelID {
  HWACCEL_NONE = 0,
  HWACCEL_AUTO,
  HWACCEL_GENERIC,
};

class InputFilter;

class InputStream : public Stream {
 public:
  InputStream(std::weak_ptr<AVFormatContext> ifmtCtx, AVStream *st,
              int file_index, unsigned int stream_index);

  ~InputStream();

  int init_input_stream();
  int guess_input_channel_layout();

 public:
  bool discard_ = false;         /* true if stream data should be discarded */
  bool decoding_needed_ = false; /* non zero if the packets must be decoded in
                          'raw_fifo', see DECODING_FOR_* */
#define DECODING_FOR_OST 1
#define DECODING_FOR_FILTER 2
  int processing_needed_ = 0; /* non zero if the packets must be processed */

  int64_t prev_pkt_pts_ = AV_NOPTS_VALUE;
  int64_t start_ = 0; /* time when read started */
  /* predicted dts of the next packet read for this stream or (when there are
   * several frames in a packet) of the next frame in current packet (in
   * AV_TIME_BASE units) */
  int64_t next_dts_ = AV_NOPTS_VALUE;
  int64_t first_dts_ = AV_NOPTS_VALUE;  ///< dts of the first packet read for
                                        ///< this stream (in AV_TIME_BASE units)
  int64_t dts_ = AV_NOPTS_VALUE;  ///< dts of the last packet read for this
                                  ///< stream (in AV_TIME_BASE units)

  int64_t next_pts_ = AV_NOPTS_VALUE;  ///< synthetic pts for the next decode
                                       ///< frame (in AV_TIME_BASE units)
  int64_t pts_ = AV_NOPTS_VALUE;  ///< current pts of the decoded frame  (in
                                  ///< AV_TIME_BASE units)
  int wrap_correction_done_ = false;

  int64_t filter_in_rescale_delta_last_ = AV_NOPTS_VALUE;

  int64_t min_pts_ =
      AV_NOPTS_VALUE; /* pts with the smallest value in a current stream */
  int64_t max_pts_ =
      AV_NOPTS_VALUE; /* pts with the higher value in a current stream */

  // when forcing constant input framerate through -r,
  // this contains the pts that will be given to the next decoded frame
  int64_t cfr_next_pts_ = AV_NOPTS_VALUE;

  int64_t nb_samples_ = AV_NOPTS_VALUE; /* number of samples in the last decoded
                        audio frame before looping */

  double ts_scale_ = 1.0;
  bool saw_first_ts_ = false;

  AVDictionary *decoder_opts_ = nullptr;
  // AVRational framerate_; /* framerate forced with -r */
  // int top_field_first_ = -1;
  int guess_layout_max_ = INT_MAX;

  int fix_sub_duration = 0;
  const AVCodec *dec_ = nullptr;
  AVDiscard user_set_discard_ = AVDISCARD_NONE;

  std::string canvas_size_;
  bool got_output_ = false;
#if 0
  struct { /* previous decoded subtitle and related variables */
    int got_output;
    int ret;
    AVSubtitle subtitle;
  } prev_sub;

  struct sub2video {
    int64_t last_pts;
    int64_t end_pts;
    AVFifo *sub_queue;  ///< queue of AVSubtitle* before filter init
    AVFrame *frame;
    int w, h;
    unsigned int initialize;  ///< marks if sub2video_update should force an
                              ///< initialization
  } sub2video;
#endif

  /* decoded data from this stream goes into all those filters
   * currently video and audio only */
  std::shared_ptr<InputFilter> filter_;
  // int nb_filters;

  // int reinit_filters_ = -1;

  /* hwaccel options */
  enum HWAccelID hwaccel_id_ = HWACCEL_NONE;
  enum AVHWDeviceType hwaccel_device_type_ = AV_HWDEVICE_TYPE_NONE;
  char *hwaccel_device_ = nullptr;
  enum AVPixelFormat hwaccel_output_format_ = AV_PIX_FMT_NONE;

  /* hwaccel context */
  void *hwaccel_ctx_ = nullptr;
  void (*hwaccel_uninit)(AVCodecContext *s);
  int (*hwaccel_retrieve_data)(AVCodecContext *s, AVFrame *frame);
  enum AVPixelFormat hwaccel_pix_fmt_ = AV_PIX_FMT_NONE;
  enum AVPixelFormat hwaccel_retrieved_pix_fmt_ = AV_PIX_FMT_NONE;

  /* stats */
  // combined size of all the packets read
  uint64_t data_size_ = 0;
  /* number of packets successfully read for this stream */
  uint64_t nb_packets_ = 0;
  // number of frames/samples retrieved from the decoder
  uint64_t frames_decoded_ = 0;
  uint64_t samples_decoded_ = 0;

  std::vector<int64_t> dts_buffer_;
  bool got_output = false;
};

}  // namespace cv
}  // namespace kingfisher

#endif
