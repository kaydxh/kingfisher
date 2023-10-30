#ifndef KINGFISHER_PKG_MIDDLEWARE_API_REQUEST_ID_H_
#define KINGFISHER_PKG_MIDDLEWARE_API_REQUEST_ID_H_

#include <string>

#include "google/protobuf/message.h"

namespace kingfisher {
namespace middleware {

std::string ExtractStringField(const ::google::protobuf::Message *message,
                               const std::string &field_name);

int RequestID(::google::protobuf::Message *message);

}  // namespace middleware
}  // namespace kingfisher

#endif
