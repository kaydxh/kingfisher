#include "request_id.h"

#include "uuid/guid.h"

namespace kingfisher {
namespace middleware {

const std::string request_id_key = "request_id";

std::string ExtractStringField(const ::google::protobuf::Message *message,
                               const std::string &field_name) {
  const ::google::protobuf::Descriptor *descriptor = message->GetDescriptor();
  const ::google::protobuf::FieldDescriptor *field =
      descriptor->FindFieldByName(field_name);

  if (field &&
      field->type() == ::google::protobuf::FieldDescriptor::TYPE_STRING) {
    return message->GetReflection()->GetString(*message, field);
  }

  return "";
}

int RequestID(::google::protobuf::Message *message) {
  auto request_id = ExtractStringField(message, request_id_key);

  const ::google::protobuf::Descriptor *descriptor = message->GetDescriptor();
  const ::google::protobuf::FieldDescriptor *field =
      descriptor->FindFieldByName(request_id_key);

  if (request_id.empty()) {
    if (field) {
      message->GetReflection()->SetString(message, field,
                                          uuid::Guid::GuidString());
      return 0;
    }
    return -1;
  }

  return 0;
}

}  // namespace middleware
}  // namespace kingfisher
