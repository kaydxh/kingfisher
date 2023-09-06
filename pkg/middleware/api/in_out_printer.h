#ifndef KINGFISHER_PKG_MIDDLEWARE_API_IN_OUT_PRINTER_H_
#define KINGFISHER_PKG_MIDDLEWARE_API_IN_OUT_PRINTER_H_

#include "google/protobuf/text_format.h"

namespace kingfisher {
namespace middleware {

void Truncate(::google::protobuf::Message *proto);

template <typename T>
std::string ProtoString(const T *proto);

}  // namespace middleware
}  // namespace kingfisher

#endif
