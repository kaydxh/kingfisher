#include "input_file.h"

#include "ffmpeg_error.h"
#include "ffmpeg_utils.h"
#include "input_stream.h"

extern "C" {
#include "libavutil/parseutils.h"
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

InputFile::~InputFile() { av_dict_free(&decoder_opts_); }

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

  /* get default parameters from command line */
  /*
  ifmt_ctx = avformat_alloc_context();
  if (!ifmt_ctx) {
    av_log(this, AV_LOG_ERROR, "failed to alloc avformat context -- %s\n",
           av_err2str(AVERROR(ENOMEM)));
    return AVERROR(ENOMEM);
  }
  */

  /*
  ifmt_ctx_->flags |= AVFMT_FLAG_NONBLOCK;
  if (bitexact_) {
    ifmt_ctx_->flags |= AVFMT_FLAG_BITEXACT;
  }
  */

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

  /*
  ifmt_ctx_ =
      std::shared_ptr<AVFormatContext>(ifmt_ctx, [](AVFormatContext *ctx) {
        //  avformat_close_input(&ctx);
      });
      */

  if (find_stream_info_) {
    /* If not enough info to get the stream parameters, we decode the
       first frames to get it. (used in mpeg case for example) */
    ret = avformat_find_stream_info(ifmt_ctx_.get(), nullptr);
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
  // add_input_streams(ifmt_ctx_.get());
  add_input_streams();

  /* dump the file content */
  av_dump_format(ifmt_ctx_.get(), 0, filename.c_str(), 0);

  return 0;
}

int InputFile::add_input_streams() {
  input_streams_.resize(ifmt_ctx_->nb_streams);
  for (unsigned int stream_id = 0; stream_id < ifmt_ctx_->nb_streams;
       stream_id++) {
    AVStream *st = ifmt_ctx_->streams[stream_id];
    std::shared_ptr<InputStream> ist = std::make_shared<InputStream>(
        std::weak_ptr(ifmt_ctx_), st, file_index_, stream_id);
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
  if (codec_name.empty()) {
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

}  // namespace cv
}  // namespace kingfisher
