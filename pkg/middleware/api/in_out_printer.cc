#include "in_out_printer.h"

#include <iostream>
#include <sstream>

namespace kingfisher {
namespace middleware {

void TruncateProto(::google::protobuf::Message *proto) {
  if (proto == nullptr) {
    return;
  }
  const ::google::protobuf::Descriptor *descriptor = proto->GetDescriptor();
  const ::google::protobuf::Reflection *reflection = proto->GetReflection();

  int field_num = descriptor->field_count();
  for (int i = 0; i < field_num; ++i) {
    const ::google::protobuf::FieldDescriptor *field = descriptor->field(i);
    if (field->is_repeated()) {
      int size = reflection->FieldSize(*proto, field);

      switch (field->type()) {
        case ::google::protobuf::FieldDescriptor::TYPE_MESSAGE: {
          for (int j = 0; j < size; ++j) {
            TruncateProto(reflection->MutableRepeatedMessage(proto, field, j));
          }
          break;
        }
        case ::google::protobuf::FieldDescriptor::TYPE_BYTES: {
          for (int j = 0; j < size; ++j) {
            std::string bytes;
            bytes = reflection->GetRepeatedStringReference(*proto, field, j,
                                                           &bytes);

            std::stringstream ss;
            ss << "bytes len:" << bytes.size();
            reflection->SetRepeatedString(proto, field, j, ss.str());
          }
          break;
        }
        default:
          break;
      }

    } else {
      switch (field->type()) {
        case ::google::protobuf::FieldDescriptor::TYPE_BYTES: {
          std::string bytes;
          bytes = reflection->GetStringReference(*proto, field, &bytes);

          std::stringstream ss;
          ss << "bytes len:" << bytes.size();
          reflection->SetString(proto, field, ss.str());
          break;
        }
        case google::protobuf::FieldDescriptor::TYPE_MESSAGE: {
          TruncateProto(reflection->MutableMessage(proto, field, nullptr));
          break;
        }
        default:
          break;
      }
    }
  }
}

}  // namespace middleware
}  // namespace kingfisher
