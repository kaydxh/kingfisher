#include "input_filter.h"

#include "ffmpeg_filter.h"
#include "stream.h"

extern "C" {
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avassert.h"
#include "libavutil/bprint.h"
#include "libavutil/fifo.h"
#include "libavutil/pixdesc.h"

static const AVClass input_filter_class = {
    .class_name = "Input Filter",
    .item_name = av_default_item_name,
    .option = nullptr,
    .version = LIBAVUTIL_VERSION_INT,
};
}

namespace kingfisher {
namespace cv {

InputFilter::InputFilter(const std::shared_ptr<FilterGraph> &fg,
                         const std::weak_ptr<Stream> &ist)
    : av_class_(&input_filter_class), graph_(fg), ist_(ist) {}

InputFilter::~InputFilter() {}

int InputFilter::ifilter_send_frame(const std::shared_ptr<AVFrame> &frame,
                                    int keep_reference) {
  int buffersrc_flags = AV_BUFFERSRC_FLAG_PUSH;
  if (keep_reference) {
    buffersrc_flags |= AV_BUFFERSRC_FLAG_KEEP_REF;
  }

  const auto &ist = ist_.lock();
  if (!ist) {
    return -1;
  }
  auto st = ist->av_stream();
  if (!st) {
    return -1;
  }

  const auto &fg = graph_.lock();

  int need_reinit = format_ != frame->format;
  switch (st->codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
      need_reinit |= sample_rate_ != frame->sample_rate ||
                     av_channel_layout_compare(&ch_layout_, &frame->ch_layout);
      break;
    case AVMEDIA_TYPE_VIDEO:
      need_reinit |= width_ != frame->width || height_ != frame->height;
      break;

    default:
      break;
  }

  if (!ist->reinit_filters_ && fg->filter_graph_) {
    need_reinit = false;
  }

  if ((hw_frames_ctx_ != nullptr) != (frame->hw_frames_ctx != nullptr) ||
      (hw_frames_ctx_ && hw_frames_ctx_->data != frame->hw_frames_ctx->data)) {
    need_reinit = true;
  }

  AVFrameSideData *sd = nullptr;
  if ((sd = av_frame_get_side_data(frame.get(), AV_FRAME_DATA_DISPLAYMATRIX))) {
    if (!display_matrix_ ||
        memcmp(sd->data, display_matrix_, sizeof(int32_t) * 9) != 0) {
      need_reinit = true;
    }
  } else if (display_matrix_) {
    need_reinit = true;
  }
  return need_reinit;
}

}  // namespace cv
}  // namespace kingfisher
