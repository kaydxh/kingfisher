#include "output_file.h"

namespace kingfisher {
namespace cv {

static const AVClass output_file_class = {
    .class_name = "OutputFile",
    .item_name = av_default_item_name,
    .option = nullptr,
    .version = LIBAVUTIL_VERSION_INT,
};

OutputFile::OutputFile() : av_class_(&output_file_class) {}
OutputFile::~OutputFile() {}

}  // namespace cv
}  // namespace kingfisher
