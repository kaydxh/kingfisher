#include "input_file.h"

#include <memory>

#include "core/scope_guard.h"
#include "ffmpeg_error.h"
#include "ffmpeg_types.h"
#include "ffmpeg_utils.h"
#include "input_stream.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/parseutils.h"
#include "libavutil/time.h"
}

namespace kingfisher {
namespace cv {

static const AVClass input_file_class = {
    .class_name = "InputFile",
    .item_name = av_default_item_name,
    .option = nullptr,
    .version = LIBAVUTIL_VERSION_INT,
};

InputFile::InputFile()
    : ifmt_ctx_(std::shared_ptr<AVFormatContext>(
          avformat_alloc_context(),
          [](AVFormatContext *ctx) { avformat_close_input(&ctx); })),
      pkt_(av_packet_alloc()),
      av_class_(&input_file_class) {
  av_dict_set(&format_opts_, "rtsp_transport", "tcp", 0);
  av_dict_set(&format_opts_, "buffer_size", "10240000", 0);
  av_dict_set(&format_opts_, "probesize", "50000000", 0);
  av_dict_set(&format_opts_, "stimeout", "5000000", 0);
  av_dict_set(&format_opts_, "max_delay", "5000000", 0);

  ifmt_ctx_->flags |= AVFMT_FLAG_NONBLOCK;
  if (bitexact_) {
    ifmt_ctx_->flags |= AVFMT_FLAG_BITEXACT;
  }
}

InputFile::~InputFile() {
  av_dict_free(&decoder_opts_);
  av_packet_free(&pkt_);
}

// https://sourcegraph.com/github.com/FFmpeg/FFmpeg@release/5.1/-/blob/fftools/ffmpeg_opt.c?L1151:59&popover=pinned
int InputFile::open(const std::string &filename, AVFormatContext &format_ctx) {
  // AVFormatContext *ifmt_ctx = nullptr;
  const AVInputFormat *file_iformat = nullptr;
  int ret = 0;
  if (!format_.empty()) {
    if (!(file_iformat = av_find_input_format(format_.c_str()))) {
      av_log(this, AV_LOG_FATAL, "Unknown input format: '%s'\n",
             format_.c_str());
      ret = AVERROR_UNKNOWN;
      return ret;
    }
  }

  bool scan_all_pmts_set = false;
  if (!av_dict_get(format_opts_, "scan_all_pmts", nullptr,
                   AV_DICT_MATCH_CASE)) {
    av_dict_set(&format_opts_, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
    scan_all_pmts_set = true;
  }

  AVFormatContext *ifmt_ctx = ifmt_ctx_.get();
  /* open the input file with generic avformat function */
  ret = avformat_open_input(&ifmt_ctx, filename.c_str(), file_iformat,
                            &format_opts_);
  if (ret < 0) {
    if (ret == AVERROR_PROTOCOL_NOT_FOUND) {
      av_log(this, AV_LOG_ERROR, "Did you mean file:%s?\n", filename.c_str());
    }
    return ret;
  }

  if (scan_all_pmts_set) {
    av_dict_set(&format_opts_, "scan_all_pmts", nullptr, AV_DICT_MATCH_CASE);
  }

  if (find_stream_info_) {
    /* If not enough info to get the stream parameters, we decode the
       first frames to get it. (used in mpeg case for example) */
    ret = avformat_find_stream_info(ifmt_ctx, nullptr);
    if (ret < 0) {
      av_log(this, AV_LOG_ERROR, "Cannot find stream information '%s': %s\n",
             filename.c_str(), av_err2str(ret));
      return ret;
    }
  }

  if (start_time_ != AV_NOPTS_VALUE && start_time_eof_ != AV_NOPTS_VALUE) {
    av_log(this, AV_LOG_WARNING,
           "Cannot use -ss and -sseof both, using -ss for %s\n",
           filename.c_str());
    start_time_eof_ = AV_NOPTS_VALUE;
  }
  if (start_time_eof_ != AV_NOPTS_VALUE) {
    if (start_time_eof_ >= 0) {
      av_log(this, AV_LOG_ERROR, "-sseof value must be negative; aborting\n");
      return AVERROR(EINVAL);
    }
    if (ifmt_ctx_->duration > 0) {
      start_time_ = start_time_eof_ + ifmt_ctx_->duration;
      if (start_time_ < 0) {
        av_log(this, AV_LOG_WARNING,
               "-sseof value seeks to before start of file %s; ignored\n",
               filename.c_str());
        start_time_ = AV_NOPTS_VALUE;
      }
    } else {
      av_log(this, AV_LOG_WARNING,
             "Cannot use -sseof, duration of %s not known\n", filename.c_str());
    }
  }

  int64_t timestamp = (start_time_ == AV_NOPTS_VALUE) ? 0 : start_time_;
  /* add the stream start time */
  if (!seek_timestamp_ && ifmt_ctx_->start_time != AV_NOPTS_VALUE) {
    timestamp += ifmt_ctx_->start_time;
  }
  /* if seeking requested, we execute it */
  if (start_time_ != AV_NOPTS_VALUE) {
    int64_t seek_timestamp = timestamp;

    if (!(ifmt_ctx_->iformat->flags & AVFMT_SEEK_TO_PTS)) {
      int dts_heuristic = 0;
      for (unsigned int i = 0; i < ifmt_ctx_->nb_streams; i++) {
        const AVCodecParameters *par = ifmt_ctx_->streams[i]->codecpar;
        if (par->video_delay) {
          dts_heuristic = 1;
          break;
        }
      }
      if (dts_heuristic) {
        seek_timestamp -= 3 * AV_TIME_BASE / 23;
      }
    }
    ret = avformat_seek_file(ifmt_ctx_.get(), -1, INT64_MIN, seek_timestamp,
                             seek_timestamp, 0);
    if (ret < 0) {
      av_log(this, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n",
             filename.c_str(), (double)timestamp / AV_TIME_BASE);
    }
  }

  /* update the current parameters so that they match the one of the input
   * stream */
  add_input_streams();

  /* dump the file content */
  av_dump_format(ifmt_ctx_.get(), 0, filename.c_str(), 0);

  return 0;
}

int InputFile::read_video_frames(std::vector<Frame> &video_frames,
                                 int32_t batch_size, bool &finished) {
  if (batch_size <= 0) {
    batch_size = 1;
  }
  auto &ist = input_streams_[first_video_stream_index_];

  SCOPE_EXIT { finished = eof_reached_ && ist->frames_.empty(); };
  int ret = read_frames([&]() {
    if (eof_reached_) {
      return true;
    }

    if (ist->frames_.size() >=
        static_cast<unsigned int>(batch_size)) {  // read batch
      return true;
    }

    return false;
  });
  if (ret < 0) {
    return ret;
  }

  if (ist->frames_.empty()) {
    return 0;
  }

  if (ist->frames_.size() <= static_cast<unsigned int>(batch_size)) {
    video_frames = std::move(ist->frames_);
  } else {
    std::copy(ist->frames_.begin(), ist->frames_.begin() + batch_size,
              video_frames.begin());
    ist->frames_.erase(ist->frames_.begin(), ist->frames_.begin() + batch_size);
  }

  return 0;
}

int InputFile::read_frames(const std::function<bool()> &stop_waiting) {
  int ret = 0;
  while (!stop_waiting()) {
    int disable_discontinuity_correction = copy_ts_;
    ret = av_read_frame(ifmt_ctx_.get(), pkt_);
    if (ret == AVERROR(EAGAIN)) {
      av_usleep(10000);
      continue;
    }

    if (ret == AVERROR_EOF) {
      av_log(nullptr, AV_LOG_VERBOSE,
             "av_read_frame all packets finished for input file #%d: %s\n",
             file_index_, av_err2str(ret));

      eof_reached_ = true;
      ret = 0;

      av_log(nullptr, AV_LOG_DEBUG,
             "Going to flush all filters for input file #%d\n", file_index_);

    } else if (ret < 0) {
      av_log(this, AV_LOG_ERROR,
             "failed to av_read_frame for input file #%d: %s\n", file_index_,
             av_err2str(ret));
      return ret;
    }

    const auto &pkt = pkt_;
    auto &ist = input_streams_[pkt_->stream_index];
    const auto is = ifmt_ctx_;
    const auto st = ist->av_stream();
    if (!ist->wrap_correction_done_ && is->start_time != AV_NOPTS_VALUE &&
        st->pts_wrap_bits < 64) {
      int64_t stime, stime2;
      // Correcting starttime based on the enabled streams
      // FIXME this ideally should be done before the first use of starttime but
      // we do not know which are the enabled streams at that point.
      //       so we instead do it here as part of discontinuity handling
      if (ist->next_dts_ == AV_NOPTS_VALUE && ts_offset_ == -is->start_time &&
          (is->iformat->flags & AVFMT_TS_DISCONT)) {
        int64_t new_start_time = INT64_MAX;
        for (unsigned int i = 0; i < is->nb_streams; i++) {
          AVStream *st = is->streams[i];
          if (st->discard == AVDISCARD_ALL ||
              st->start_time == AV_NOPTS_VALUE) {
            continue;
          }
          new_start_time = FFMIN(
              new_start_time,
              av_rescale_q(st->start_time, st->time_base, AV_TIME_BASE_Q));
        }
        if (new_start_time > is->start_time) {
          av_log(is.get(), AV_LOG_VERBOSE,
                 "Correcting start time by %" PRId64 "\n",
                 new_start_time - is->start_time);
          ts_offset_ = -new_start_time;
        }
      }

      stime = av_rescale_q(is->start_time, AV_TIME_BASE_Q, st->time_base);
      stime2 = stime + (1ULL << st->pts_wrap_bits);
      ist->wrap_correction_done_ = 1;

      if (stime2 > stime && pkt->dts != AV_NOPTS_VALUE &&
          pkt->dts > stime + (1LL << (st->pts_wrap_bits - 1))) {
        pkt->dts -= 1ULL << st->pts_wrap_bits;
        ist->wrap_correction_done_ = 0;
      }
      if (stime2 > stime && pkt->pts != AV_NOPTS_VALUE &&
          pkt->pts > stime + (1LL << (st->pts_wrap_bits - 1))) {
        pkt->pts -= 1ULL << st->pts_wrap_bits;
        ist->wrap_correction_done_ = 0;
      }
    }

    /* add the stream-global side data to the first packet */
    if (ist->nb_packets_ == 1) {
      // todo
    }

    if (pkt->dts != AV_NOPTS_VALUE) {
      pkt->dts += av_rescale_q(ts_offset_, AV_TIME_BASE_Q, st->time_base);
    }
    if (pkt->pts != AV_NOPTS_VALUE) {
      pkt->pts += av_rescale_q(ts_offset_, AV_TIME_BASE_Q, st->time_base);
    }

    if (pkt->pts != AV_NOPTS_VALUE) {
      pkt->pts *= ist->ts_scale_;
    }
    if (pkt->dts != AV_NOPTS_VALUE) {
      pkt->dts *= ist->ts_scale_;
    }

    int64_t pkt_dts = av_rescale_q_rnd(
        pkt->dts, st->time_base, AV_TIME_BASE_Q,
        static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    if ((ist->codec_ctx_->codec_type == AVMEDIA_TYPE_VIDEO ||
         ist->codec_ctx_->codec_type == AVMEDIA_TYPE_AUDIO) &&
        pkt_dts != AV_NOPTS_VALUE && ist->next_dts_ == AV_NOPTS_VALUE &&
        !copy_ts_ && (is->iformat->flags & AVFMT_TS_DISCONT) &&
        last_ts_ != AV_NOPTS_VALUE) {
      int64_t delta = pkt_dts - last_ts_;
      if (delta < -1LL * dts_delta_threshold_ * AV_TIME_BASE ||
          delta > 1LL * dts_delta_threshold_ * AV_TIME_BASE) {
        ts_offset_ -= delta;
        av_log(this, AV_LOG_DEBUG,
               "Inter stream timestamp discontinuity %" PRId64
               ", new offset= %" PRId64 "\n",
               delta, ts_offset_);
        pkt->dts -= av_rescale_q(delta, AV_TIME_BASE_Q, st->time_base);
        if (pkt->pts != AV_NOPTS_VALUE) {
          pkt->pts -= av_rescale_q(delta, AV_TIME_BASE_Q, st->time_base);
        }
      }
    }

    int64_t duration = av_rescale_q(duration_, time_base_, st->time_base);
    if (pkt->pts != AV_NOPTS_VALUE) {
      pkt->pts += duration;
      ist->max_pts_ = FFMAX(pkt->pts, ist->max_pts_);
      ist->min_pts_ = FFMIN(pkt->pts, ist->min_pts_);
    }

    if (pkt->dts != AV_NOPTS_VALUE) {
      pkt->dts += duration;
    }

    pkt_dts = av_rescale_q_rnd(
        pkt->dts, st->time_base, AV_TIME_BASE_Q,
        static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

    if (copy_ts_ && pkt_dts != AV_NOPTS_VALUE &&
        ist->next_dts_ != AV_NOPTS_VALUE &&
        (is->iformat->flags & AVFMT_TS_DISCONT) && st->pts_wrap_bits < 60) {
      int64_t wrap_dts = av_rescale_q_rnd(
          pkt->dts + (1LL << st->pts_wrap_bits), st->time_base, AV_TIME_BASE_Q,
          static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
      if (FFABS(wrap_dts - ist->next_dts_) <
          FFABS(pkt_dts - ist->next_dts_) / 10) {
        disable_discontinuity_correction = 0;
      }
    }

    if ((ist->codec_ctx_->codec_type == AVMEDIA_TYPE_VIDEO ||
         ist->codec_ctx_->codec_type == AVMEDIA_TYPE_AUDIO) &&
        pkt_dts != AV_NOPTS_VALUE && ist->next_dts_ != AV_NOPTS_VALUE &&
        !disable_discontinuity_correction) {
      int64_t delta = pkt_dts - ist->next_dts_;
      if (is->iformat->flags & AVFMT_TS_DISCONT) {
        if (delta < -1LL * dts_delta_threshold_ * AV_TIME_BASE ||
            delta > 1LL * dts_delta_threshold_ * AV_TIME_BASE ||
            pkt_dts + AV_TIME_BASE / 10 < FFMAX(ist->pts_, ist->dts_)) {
          ts_offset_ -= delta;
          av_log(nullptr, AV_LOG_DEBUG,
                 "timestamp discontinuity for stream #%d:%d "
                 "(id=%d, type=%s): %" PRId64 ", new offset= %" PRId64 "\n",
                 ist->file_index_, st->index, st->id,
                 av_get_media_type_string(ist->codec_ctx_->codec_type), delta,
                 ts_offset_);
          pkt->dts -= av_rescale_q(delta, AV_TIME_BASE_Q, st->time_base);
          if (pkt->pts != AV_NOPTS_VALUE) {
            pkt->pts -= av_rescale_q(delta, AV_TIME_BASE_Q, st->time_base);
          }
        }
      } else {
        if (delta < -1LL * dts_error_threshold_ * AV_TIME_BASE ||
            delta > 1LL * dts_error_threshold_ * AV_TIME_BASE) {
          av_log(nullptr, AV_LOG_WARNING,
                 "DTS %" PRId64 ", next:%" PRId64 " st:%d invalid dropping\n",
                 pkt->dts, ist->next_dts_, pkt->stream_index);
          pkt->dts = AV_NOPTS_VALUE;
        }
        if (pkt->pts != AV_NOPTS_VALUE) {
          int64_t pkt_pts =
              av_rescale_q(pkt->pts, st->time_base, AV_TIME_BASE_Q);
          delta = pkt_pts - ist->next_dts_;
          if (delta < -1LL * dts_error_threshold_ * AV_TIME_BASE ||
              delta > 1LL * dts_error_threshold_ * AV_TIME_BASE) {
            av_log(nullptr, AV_LOG_WARNING,
                   "PTS %" PRId64 ", next:%" PRId64 " invalid dropping st:%d\n",
                   pkt->pts, ist->next_dts_, pkt->stream_index);
            pkt->pts = AV_NOPTS_VALUE;
          }
        }
      }
    }

    if (pkt->dts != AV_NOPTS_VALUE) {
      last_ts_ = av_rescale_q(pkt->dts, st->time_base, AV_TIME_BASE_Q);
    }
    ret = process_input_packet(ist, pkt, true);
    if (ret < 0 && ret != AVERROR_EOF) {
      return ret;
    }
  }

  return 0;
}

int InputFile::add_input_streams() {
  input_streams_.resize(ifmt_ctx_->nb_streams);
  for (unsigned int stream_id = 0; stream_id < ifmt_ctx_->nb_streams;
       stream_id++) {
    bool discard_stream = false;
    AVStream *st = ifmt_ctx_->streams[stream_id];
    std::shared_ptr<InputStream> ist =
        std::make_shared<InputStream>(ifmt_ctx_, st, file_index_, stream_id);
    ist->discard_ = AVDISCARD_DEFAULT;
    st->discard = AVDISCARD_ALL;

    int ret = match_per_stream_opt(this, command_opts_, ifmt_ctx_.get(), st,
                                   "autorotate", ist->autorotate_);
    if (ret != 0) {
      return ret;
    }
    ret = match_per_stream_opt(this, command_opts_, ifmt_ctx_.get(), st,
                               "ts_scale", ist->ts_scale_);
    if (ret != 0) {
      return ret;
    }

    const AVCodec *dec = nullptr;
    ret = choose_decoder(ist, dec);
    if (ret != 0) {
      return ret;
    }
    ist->dec_ = dec;
    ist->decoder_opts_ = filter_codec_opts(
        decoder_opts_, st->codecpar->codec_id, ifmt_ctx_.get(), st, ist->dec_);

    if ((video_disable_ && st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) ||
        (audio_disable_ && st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) ||
        (subtitle_disable_ &&
         st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) ||
        (data_disable_ && st->codecpar->codec_type == AVMEDIA_TYPE_DATA)) {
      ist->user_set_discard_ = AVDISCARD_ALL;
    }

    ist->filter_in_rescale_delta_last_ = AV_NOPTS_VALUE;
    ist->prev_pkt_pts_ = AV_NOPTS_VALUE;

    ist->codec_ctx_ = std::shared_ptr<AVCodecContext>(
        avcodec_alloc_context3(ist->dec_),
        [](AVCodecContext *avctx) { avcodec_free_context(&avctx); });
    if (!ist->codec_ctx_) {
      av_log(this, AV_LOG_ERROR,
             "failed to allocate the decoder context#%d:%d\n", file_index_,
             stream_id);
      return AVERROR(ENOMEM);
    }

    ret = avcodec_parameters_to_context(ist->codec_ctx_.get(), st->codecpar);
    if (ret < 0) {
      av_log(this, AV_LOG_ERROR,
             "failed to initialize the decoder context stream#%d:%d -- %s\n",
             file_index_, stream_id, av_err2str(ret));
      return ret;
    }
    if (bitexact_) {
      ist->codec_ctx_->flags |= AV_CODEC_FLAG_BITEXACT;
    }

    AVCodecParameters *par = st->codecpar;
    switch (par->codec_type) {
      case AVMEDIA_TYPE_VIDEO:
        if (first_video_stream_index_ >= 0) {
          discard_stream = true;
          break;
        }

        first_video_stream_index_ = stream_id;
        if (!ist->dec_) {
          ist->dec_ = avcodec_find_decoder(par->codec_id);
        }

        // avformat_find_stream_info() doesn't set this for us anymore.
        ist->codec_ctx_->framerate = st->avg_frame_rate;

        /*
        if (framerate_ &&
            av_parse_video_rate(&ist->framerate_, framerate) < 0) {
          av_log(NULL, AV_LOG_ERROR, "Error parsing framerate %s.\n",
                 framerate);
          exit_program(1);
        }
        */
        if (!ist->codec_ctx_->framerate.num) {
          ist->codec_ctx_->framerate =
              av_guess_frame_rate(ifmt_ctx_.get(), st, nullptr);
        }
        // TODO support hwcaael options
        break;
      case AVMEDIA_TYPE_AUDIO:
        if (first_audio_stream_index_ >= 0) {
          discard_stream = true;
          break;
        }
        first_audio_stream_index_ = stream_id;
        ist->guess_input_channel_layout();
        break;
      case AVMEDIA_TYPE_DATA:
      case AVMEDIA_TYPE_SUBTITLE: {
        if (!ist->dec_) {
          ist->dec_ = avcodec_find_decoder(par->codec_id);
        }
        if (!ist->canvas_size_.empty() &&
            av_parse_video_size(&ist->codec_ctx_->width,
                                &ist->codec_ctx_->height,
                                ist->canvas_size_.c_str()) < 0) {
          av_log(this, AV_LOG_FATAL, "Invalid canvas size: %s.\n",
                 ist->canvas_size_.c_str());
          return AVERROR(EINVAL);
        }
        break;
      }
      case AVMEDIA_TYPE_ATTACHMENT:
      case AVMEDIA_TYPE_UNKNOWN:
        break;
      default:
        break;
    }

    if (discard_stream) {
      continue;
    }

    ret = avcodec_parameters_from_context(par, ist->codec_ctx_.get());
    if (ret < 0) {
      av_log(this, AV_LOG_ERROR, "Error initializing the decoder context.\n");
      return ret;
    }

    /* init input streams */
    ret = ist->init_input_stream();
    if (ret < 0) {
      av_log(this, AV_LOG_ERROR, "Failed to init input stream #%d:%d -- %s\n",
             file_index_, stream_id, av_err2str(ret));
      return ret;
    }

    input_streams_[stream_id] = ist;
  }

  return 0;
}

int InputFile::choose_decoder(const std::shared_ptr<InputStream> &ist,
                              const AVCodec *&codec) {
  std::string codec_name;
  auto &st = ist->st_;
  /*
  int ret = match_per_stream_opt(this, command_opts_, ifmt_ctx_.get(), st.get(),
                                 "c", codec_name);
  if (ret != 0) {
    return ret;
  }
  */
  if (!codec_name.empty()) {
    int ret = find_decoder(codec_name, st->codecpar->codec_type, codec);
    if (ret) {
      return ret;
    }
    st->codecpar->codec_id = codec->id;
    if (recast_media_ && st->codecpar->codec_type != codec->type) {
      st->codecpar->codec_type = codec->type;
    }
    return 0;
  } else {
    codec = avcodec_find_decoder(st->codecpar->codec_id);
  }
  return 0;
}

int InputFile::find_decoder(const std::string &name, enum AVMediaType type,
                            const AVCodec *&codec) const {
  return find_codec((void *)this, name, type, false, codec, recast_media_);
}

// https://sourcegraph.com/github.com/FFmpeg/FFmpeg@release/5.1/-/blob/fftools/ffmpeg.c?//L2367:12&popover=pinned
int InputFile::process_input_packet(const std::shared_ptr<InputStream> &ist,
                                    AVPacket *pkt, int no_eof) {
  std::shared_ptr<AVPacket> avpkt = ist->pkt_;
  AVStream *st = ist->av_stream();
  if (!ist->saw_first_ts_) {
    ist->first_dts_ = ist->dts_ = ist->codec_ctx_ && st->avg_frame_rate.num
                                      ? -ist->codec_ctx_->has_b_frames *
                                            AV_TIME_BASE /
                                            av_q2d(st->avg_frame_rate)
                                      : 0;
    ist->pts_ = 0;
    if (pkt && pkt->pts != AV_NOPTS_VALUE && !ist->decoding_needed_) {
      ist->first_dts_ = ist->dts_ +=
          av_rescale_q(pkt->pts, st->time_base, AV_TIME_BASE_Q);
      ist->pts_ = ist->dts_;  // unused but better to set it to a value thats
                              // not totally wrong
    }
    ist->saw_first_ts_ = true;
  }

  if (ist->next_dts_ == AV_NOPTS_VALUE) {
    ist->next_dts_ = ist->dts_;
  }
  if (ist->next_pts_ == AV_NOPTS_VALUE) {
    ist->next_pts_ = ist->pts_;
  }

  int ret = 0;
  if (pkt) {
    av_packet_unref(avpkt.get());
    ret = av_packet_ref(avpkt.get(), pkt);
    if (ret < 0) {
      return ret;
    }
  }

  bool repeating = false;
  bool eof_reached = false;  // meet decode eof
  // while we have more to decode or while the decoder did output something on
  // EOF
  while (ist->decoding_needed_) {
    int64_t duration_dts = 0;
    int64_t duration_pts = 0;
    bool got_output = false;
    bool decode_failed = false;

    ist->pts_ = ist->next_pts_;
    ist->dts_ = ist->next_dts_;

    switch (ist->codec_ctx_->codec_type) {
      case AVMEDIA_TYPE_AUDIO:
        ret = decode_audio(ist, repeating ? nullptr : avpkt.get(), got_output,
                           decode_failed);
        av_packet_unref(avpkt.get());
        break;
      case AVMEDIA_TYPE_VIDEO:
        ret = decode_video(ist, repeating ? nullptr : avpkt.get(), !pkt,
                           got_output, duration_pts, decode_failed);
        if (!repeating || !pkt || got_output) {
          if (pkt && pkt->duration) {
            duration_dts =
                av_rescale_q(pkt->duration, st->time_base, AV_TIME_BASE_Q);
          } else if (ist->codec_ctx_->framerate.num != 0 &&
                     ist->codec_ctx_->framerate.den != 0) {
            int ticks = av_stream_get_parser(st)
                            ? av_stream_get_parser(st)->repeat_pict + 1
                            : ist->codec_ctx_->ticks_per_frame;
            duration_dts = ((int64_t)AV_TIME_BASE *
                            ist->codec_ctx_->framerate.den * ticks) /
                           ist->codec_ctx_->framerate.num /
                           ist->codec_ctx_->ticks_per_frame;
          }

          if (ist->dts_ != AV_NOPTS_VALUE && duration_dts) {
            ist->next_dts_ += duration_dts;
          } else {
            ist->next_dts_ = AV_NOPTS_VALUE;
          }
        }

        if (got_output) {
          if (duration_pts > 0) {
            ist->next_pts_ +=
                av_rescale_q(duration_pts, st->time_base, AV_TIME_BASE_Q);
          } else {
            ist->next_pts_ += duration_dts;
          }
        }
        av_packet_unref(avpkt.get());
        break;
      default:
        return -1;
    }

    if (ret == AVERROR_EOF) {
      eof_reached = 1;
      break;
    }

    if (ret < 0) {
      if (decode_failed) {
        av_log(this, AV_LOG_ERROR, "Error while decoding stream #%d:%d: %s\n",
               ist->file_index_, ist->stream_index_, av_err2str(ret));
      } else {
        av_log(this, AV_LOG_FATAL,
               "Error while processing the decoded "
               "data for stream #%d:%d\n",
               ist->file_index_, ist->stream_index_);
      }
      if (!decode_failed) {
        return ret;
      }
      break;
    }

    if (got_output) {
      ist->got_output_ = true;
    }

    if (!got_output) {
      break;
    }

    // During draining, we might get multiple output frames in this loop.
    // ffmpeg.c does not drain the filter chain on configuration changes,
    // which means if we send multiple frames at once to the filters, and
    // one of those frames changes configuration, the buffered frames will
    // be lost. This can upset certain FATE tests.
    // Decode only 1 frame per call on EOF to appease these FATE tests.
    // The ideal solution would be to rewrite decoding to use the new
    // decoding API in a better way.
    // FIXME: we will drain all frames if no more packets will be provided, that
    // is av_read_frame eof
    if (!pkt && no_eof) {
      break;
    }

    repeating = 1;
  }

  /* after flushing, send an EOF on all the filter inputs attached to the stream
   */
  /* except when looping we need to flush but not to send an EOF */
  if (!pkt && ist->decoding_needed_ && eof_reached && !no_eof) {
    ret = send_filter_eof(ist);
    if (ret < 0) {
      av_log(this, AV_LOG_FATAL, "Error marking filters as finished\n");
      return ret;
    }
  }

  /* handle stream copy */
  if (!ist->decoding_needed_ && ist->codec_ctx_) {
    if (pkt) {
      ist->dts_ = ist->next_dts_;
      switch (ist->codec_ctx_->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
          av_assert1(pkt->duration >= 0);
          if (ist->codec_ctx_->sample_rate) {
            ist->next_dts_ +=
                ((int64_t)AV_TIME_BASE * ist->codec_ctx_->frame_size) /
                ist->codec_ctx_->sample_rate;
          } else {
            ist->next_dts_ +=
                av_rescale_q(pkt->duration, st->time_base, AV_TIME_BASE_Q);
          }
          break;
        case AVMEDIA_TYPE_VIDEO:
          if (ist->framerate_.num) {
            // TODO: Remove work-around for c99-to-c89 issue 7
            AVRational time_base_q = AV_TIME_BASE_Q;
            int64_t next_dts = av_rescale_q(ist->next_dts_, time_base_q,
                                            av_inv_q(ist->framerate_));
            ist->next_dts_ = av_rescale_q(
                next_dts + 1, av_inv_q(ist->framerate_), time_base_q);
          } else if (pkt->duration) {
            ist->next_dts_ +=
                av_rescale_q(pkt->duration, st->time_base, AV_TIME_BASE_Q);
          } else if (ist->codec_ctx_->framerate.num != 0) {
            int ticks = av_stream_get_parser(st)
                            ? av_stream_get_parser(st)->repeat_pict + 1
                            : ist->codec_ctx_->ticks_per_frame;
            ist->next_dts_ += ((int64_t)AV_TIME_BASE *
                               ist->codec_ctx_->framerate.den * ticks) /
                              ist->codec_ctx_->framerate.num /
                              ist->codec_ctx_->ticks_per_frame;
          }
          break;
        default:
          break;
      }
      ist->pts_ = ist->dts_;
      ist->next_pts_ = ist->next_dts_;
    } else {
      eof_reached = 1;
    }
    ret = stream_copy(ist, pkt);
    if (ret < 0) {
      return ret;
    }
  }

  return !eof_reached;
}

int InputFile::stream_copy(const std::shared_ptr<InputStream> &ist,
                           AVPacket *packet) {
  AVStream *st = ist->av_stream();
  if (!st) {
    return -1;
  }

  if (packet) {
    ist->frame_number_++;
    Frame raw_frame;
    raw_frame.time_base = packet->time_base;
    raw_frame.pts = packet->pts;
    raw_frame.frame_number = ist->frame_number_;
    raw_frame.codec_id = st->codecpar->codec_id;
    raw_frame.codec_type = st->codecpar->codec_type;
    raw_frame.packet = std::shared_ptr<AVPacket>(
        av_packet_clone(packet), [](AVPacket *pkt) { av_packet_free(&pkt); });
    switch (ist->codec_ctx_->codec_type) {
      case AVMEDIA_TYPE_VIDEO:
        ist->frames_.push_back(raw_frame);
        break;
      default:
        return 0;
    }
  }
  return 0;
}

int InputFile::decode_video(const std::shared_ptr<InputStream> &ist,
                            AVPacket *pkt, int eof, bool &got_output,
                            int64_t &duration_pts, bool &decode_failed) {
  std::shared_ptr<AVPacket> avpkt = ist->pkt_;
  std::shared_ptr<AVFrame> decoded_frame = ist->frame_;
  AVStream *st = ist->av_stream();
  int64_t dts = AV_NOPTS_VALUE;

  // With fate-indeo3-2, we're getting 0-sized packets before EOF for some
  // reason. This seems like a semi-critical bug. Don't trigger EOF, and
  // skip the packet.
  if (!eof && pkt && pkt->size == 0) {
    return 0;
  }

  if (ist->dts_ != AV_NOPTS_VALUE) {
    dts = av_rescale_q(ist->dts_, AV_TIME_BASE_Q, st->time_base);
  }
  if (pkt) {
    pkt->dts = dts;  // ffmpeg.c probably shouldn't do this
  }

  // The old code used to set dts on the drain packet, which does not work
  // with the new API anymore.
  if (eof) {
    ist->dts_buffer_.emplace_back(dts);
  }

  int ret = decode(ist->codec_ctx_.get(), pkt, decoded_frame.get(), got_output);
  if (ret < 0) {
    decode_failed = true;
  }

  // The following line may be required in some cases where there is no parser
  // or the parser does not has_b_frames correctly
  if (st->codecpar->video_delay < ist->codec_ctx_->has_b_frames) {
    if (ist->codec_ctx_->codec_id == AV_CODEC_ID_H264) {
      st->codecpar->video_delay = ist->codec_ctx_->has_b_frames;
    } else
      av_log(ist->codec_ctx_.get(), AV_LOG_WARNING,
             "video_delay is larger in decoder than demuxer %d > %d.\n"
             "If you want to help, upload a sample "
             "of this file to https://streams.videolan.org/upload/ "
             "and contact the ffmpeg-devel mailing list. "
             "(ffmpeg-devel@ffmpeg.org)\n",
             ist->codec_ctx_->has_b_frames, st->codecpar->video_delay);
  }

  if (ret != AVERROR_EOF) {
    ret = check_decode_result(ist, got_output);
    if (ret < 0) {
      return ret;
    }
  }

  if (got_output && ret >= 0) {
    if (ist->codec_ctx_->width != decoded_frame->width ||
        ist->codec_ctx_->height != decoded_frame->height ||
        ist->codec_ctx_->pix_fmt != decoded_frame->format) {
      av_log(this, AV_LOG_DEBUG,
             "Frame parameters mismatch context %d,%d,%d != %d,%d,%d\n",
             decoded_frame->width, decoded_frame->height, decoded_frame->format,
             ist->codec_ctx_->width, ist->codec_ctx_->height,
             ist->codec_ctx_->pix_fmt);
    }
  }

  if (!got_output || ret < 0) {
    return ret;
  }

  if (ist->top_field_first_ >= 0) {
    decoded_frame->top_field_first = ist->top_field_first_;
  }

  ist->frames_decoded_++;

  int64_t best_effort_timestamp = decoded_frame->best_effort_timestamp;
  duration_pts = decoded_frame->pkt_duration;
  if (ist->framerate_.num) {
    best_effort_timestamp = ist->cfr_next_pts_++;
  }

  if (eof && best_effort_timestamp == AV_NOPTS_VALUE &&
      !ist->dts_buffer_.empty()) {
    best_effort_timestamp = ist->dts_buffer_[0];
    ist->dts_buffer_.erase(ist->dts_buffer_.begin());
  }

  if (best_effort_timestamp != AV_NOPTS_VALUE) {
    int64_t ts = av_rescale_q(decoded_frame->pts = best_effort_timestamp,
                              st->time_base, AV_TIME_BASE_Q);

    if (ts != AV_NOPTS_VALUE) {
      ist->next_pts_ = ist->pts_ = ts;
    }
  }

  if (debug_ts_) {
    av_log(
        this, AV_LOG_DEBUG,
        "detail: decoder -> ist_index:%d type:video "
        "frame_pts:%s frame_pts_time:%s best_effort_ts:%" PRId64
        " best_effort_ts_time:%s keyframe:%d frame_type:%d time_base:%d/%d\n",
        st->index, av_err2str(decoded_frame->pts),
        av_ts2timestr(decoded_frame->pts, &st->time_base),
        best_effort_timestamp,
        av_ts2timestr(best_effort_timestamp, &st->time_base),
        decoded_frame->key_frame, decoded_frame->pict_type, st->time_base.num,
        st->time_base.den);
  }

  if (st->sample_aspect_ratio.num) {
    decoded_frame->sample_aspect_ratio = st->sample_aspect_ratio;
  }

  // todo
  //  err = send_frame_to_filters(ist, decoded_frame);
  av_frame_unref(decoded_frame.get());
  return ret;
}

int InputFile::check_decode_result(const std::shared_ptr<InputStream> &ist,
                                   bool &got_output) {
  if (got_output && ist) {
    if (ist->frame_->decode_error_flags ||
        (ist->frame_->flags & AV_FRAME_FLAG_CORRUPT)) {
      av_log(this, AV_LOG_WARNING, "%s: corrupt decoded frame in stream %d\n",
             ifmt_ctx_->url, ist->st_->index);
      return AVERROR_INVALIDDATA;
    }
  }

  return 0;
}

int InputFile::decode_audio(const std::shared_ptr<InputStream> &ist,
                            AVPacket *pkt, bool &got_output,
                            bool &decode_failed) {
  std::shared_ptr<AVFrame> decoded_frame = ist->frame_;
  std::shared_ptr<AVCodecContext> avctx = ist->codec_ctx_;
  int ret = decode(avctx.get(), pkt, decoded_frame.get(), got_output);
  if (ret < 0) {
    decode_failed = true;
  }

  if (ret >= 0 && avctx->sample_rate <= 0) {
    av_log(avctx.get(), AV_LOG_ERROR, "Sample rate %d invalid\n",
           avctx->sample_rate);
    ret = AVERROR_INVALIDDATA;
  }

  if (ret != AVERROR_EOF) {
    ret = check_decode_result(ist, got_output);
    if (ret < 0) {
      return ret;
    }
  }

  if (!got_output || ret < 0) {
    return ret;
  }
  ist->samples_decoded_ += decoded_frame->nb_samples;
  ist->frames_decoded_++;

  /* increment next_dts to use for the case where the input stream does not
   have timestamps or there are multiple frames in the packet */
  ist->next_pts_ +=
      ((int64_t)AV_TIME_BASE * decoded_frame->nb_samples) / avctx->sample_rate;
  ist->next_dts_ +=
      ((int64_t)AV_TIME_BASE * decoded_frame->nb_samples) / avctx->sample_rate;

  AVRational decoded_frame_tb;
  if (decoded_frame->pts != AV_NOPTS_VALUE) {
    decoded_frame_tb = ist->st_->time_base;
  } else if (pkt && pkt->pts != AV_NOPTS_VALUE) {
    decoded_frame->pts = pkt->pts;
    decoded_frame_tb = ist->st_->time_base;
  } else {
    decoded_frame->pts = ist->dts_;
    decoded_frame_tb = AV_TIME_BASE_Q;
  }
  if (pkt && pkt->duration && ist->prev_pkt_pts_ != AV_NOPTS_VALUE &&
      pkt->pts != AV_NOPTS_VALUE &&
      pkt->pts - ist->prev_pkt_pts_ > pkt->duration) {
    ist->filter_in_rescale_delta_last_ = AV_NOPTS_VALUE;
  }
  if (pkt) {
    ist->prev_pkt_pts_ = pkt->pts;
  }
  if (decoded_frame->pts != AV_NOPTS_VALUE) {
    decoded_frame->pts = av_rescale_delta(decoded_frame_tb, decoded_frame->pts,
                                          (AVRational){1, avctx->sample_rate},
                                          decoded_frame->nb_samples,
                                          &ist->filter_in_rescale_delta_last_,
                                          (AVRational){1, avctx->sample_rate});
  }

  ist->nb_samples_ = decoded_frame->nb_samples;
  // todo
  // err = send_frame_to_filters(ist, decoded_frame);
  av_frame_unref(decoded_frame.get());

  return ret;
}

// This does not quite work like avcodec_decode_audio4/avcodec_decode_video2.
// There is the following difference: if you got a frame, you must call
// it again with pkt=NULL. pkt==NULL is treated differently from pkt->size==0
// (pkt==NULL means get more output, pkt->size==0 is a flush/drain packet)
int InputFile::decode(AVCodecContext *avctx, AVPacket *pkt, AVFrame *frame,
                      bool &got_frame) {
  int ret = 0;
  got_frame = false;

  if (pkt) {
    ret = avcodec_send_packet(avctx, pkt);
    // In particular, we don't expect AVERROR(EAGAIN), because we read all
    // decoded frames with avcodec_receive_frame() until done.
    if (ret < 0 && ret != AVERROR_EOF) {
      return ret;
    }
  }

  ret = avcodec_receive_frame(avctx, frame);
  if (ret < 0 && ret != AVERROR(EAGAIN)) {
    return ret;
  }
  if (ret >= 0) {
    got_frame = true;
  }

  return 0;
}

int InputFile::send_filter_eof(const std::shared_ptr<InputStream> &ist) {
  AVStream *st = ist->av_stream();
  // int i = 0;
  // int ret = 0;
  /* TODO keep pts also in stream time base to avoid converting back */
  int64_t pts = av_rescale_q_rnd(
      ist->pts_, AV_TIME_BASE_Q, st->time_base,
      static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

  return ist->filter_->ifilter_send_eof(pts);
}

int InputFile::send_frame_to_filters(
    const std::shared_ptr<InputStream> &ist,
    const std::shared_ptr<AVFrame> &decoded_frame) {
  // const auto &stream_index = ist->stream_index_;
  //
  return 0;
}

}  // namespace cv
}  // namespace kingfisher
