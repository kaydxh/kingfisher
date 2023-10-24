#ifndef KINGFISHER_PKG_MIDDLEWARE_API_IN_OUT_PRINTER_H_
#define KINGFISHER_PKG_MIDDLEWARE_API_IN_OUT_PRINTER_H_

#include "google/protobuf/text_format.h"

namespace kingfisher {
namespace middleware {

void TruncateProto(::google::protobuf::Message *proto);

template <typename T>
std::string ProtoString(const T * proto){
  T cloneProto;
  cloneProto.CopyFrom(*proto);
  TruncateProto(&cloneProto);

  ::google::protobuf::TextFormat::Printer printer;
  printer.SetSingleLineMode(true);
  std::string output;
  printer.PrintToString(cloneProto, &output);
  return output;
}

}  // namespace middleware
}  // namespace kingfisher

#endif
