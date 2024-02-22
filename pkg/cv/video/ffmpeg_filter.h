#ifndef KINGFISHER_PKG_CV_VIDEO_FFMPEG_FILTER_H_
#define KINGFISHER_PKG_CV_VIDEO_FFMPEG_FILTER_H_

#include <memory>
#include <vector>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
};

namespace kingfisher {
namespace cv {

class InputFilter;
class OutputFilter;
class Stream;

class FilterGraph {
 public:
  FilterGraph();
  ~FilterGraph();

  int filtergraph_is_simple();
  void cleanup_filtergraph();
  int configure_filtergraph();
  int reap_filters();

  static int insert_filter(AVFilterContext **last_filter, int *pad_idx,
                           const char *filter_name, const char *args);
  static double get_rotation(int32_t *displaymatrix);

 public:
  std::shared_ptr<AVFilterGraph> filter_graph_;
  std::vector<std::shared_ptr<InputFilter>> inputs_;
  std::vector<std::shared_ptr<OutputFilter>> outputs_;

  std::weak_ptr<Stream> output_stream_;

  std::string graph_desc_;
  int reconfiguration_ = 0;
  // true when the filtergraph contains only meta filters
  // that do not modify the frame data
  bool is_meta_ = false;
  int filter_nbthreads_ = 0;
};

}  // namespace cv
}  // namespace kingfisher

#endif
