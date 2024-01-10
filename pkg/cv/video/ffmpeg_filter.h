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

class FilterGraph {
 public:
  FilterGraph();
  ~FilterGraph();

 public:
  std::shared_ptr<AVFilterGraph> filter_graph_;
  std::vector<std::shared_ptr<InputFilter>> inputs_;
  std::vector<std::shared_ptr<OutputFilter>> outputs_;

  std::string graph_desc_;
  int reconfiguration_ = 0;
  // true when the filtergraph contains only meta filters
  // that do not modify the frame data
  int is_meta_ = 0;
};

}  // namespace cv
}  // namespace kingfisher

#endif
