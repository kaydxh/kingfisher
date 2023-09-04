#include "yaml.h"

namespace kingfisher {
namespace yaml {

YAML::Node GetYaml(const std::string& path) {
  YAML::Node data = YAML::LoadFile(path);
  return data;
}

int UnmarshalProtoMessage(const YAML::Node& yaml_node,
                          google::protobuf::Message& message) {
  const google::protobuf::Reflection* reflection = message.GetReflection();
  const google::protobuf::Descriptor* descriptor = message.GetDescriptor();

  for (int i = 0; i < descriptor->field_count(); ++i) {
    const google::protobuf::FieldDescriptor* field = descriptor->field(i);
    const std::string& field_name = field->name();

    if (yaml_node[field_name] && yaml_node[field_name].IsMap()) {
      switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
          const YAML::Node& sub_node = yaml_node[field_name];
          if (sub_node.IsMap()) {
            google::protobuf::Message* sub_message =
                reflection->MutableMessage(&message, field);
            if (sub_message != nullptr) {
              UnmarshalProtoMessage(sub_node, *sub_message);
            }
          }
          break;
        }
        default:
          break;
      }
    } else {
      if (field->is_repeated()) {
        // Handle repeated fields
        if (yaml_node[field_name].IsSequence()) {
          for (const auto& item : yaml_node[field_name]) {
            setValue(item, reflection, field, message, true);
          }
        }
      } else {
        setValue(yaml_node[field_name], reflection, field, message, false);
      }
    }
  }

  return 0;
}

int setValue(const YAML::Node& yaml_node,
             const google::protobuf::Reflection* reflection,
             const google::protobuf::FieldDescriptor* field,
             google::protobuf::Message& message, bool add) {
  switch (field->cpp_type()) {
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
      if (add) {
        reflection->AddBool(&message, field, yaml_node.as<bool>());
      } else {
        reflection->SetBool(&message, field, yaml_node.as<bool>());
      }
      break;
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
      if (add) {
        reflection->AddInt32(&message, field, yaml_node.as<int32_t>());
      } else {
        reflection->SetInt32(&message, field, yaml_node.as<int32_t>());
      }
      break;
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
      if (add) {
        reflection->AddInt64(&message, field, yaml_node.as<int64_t>());
      } else {
        reflection->SetInt64(&message, field, yaml_node.as<int64_t>());
      }
      break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
      if (add) {
        reflection->AddUInt32(&message, field, yaml_node.as<uint32_t>());
      } else {
        reflection->SetUInt32(&message, field, yaml_node.as<uint32_t>());
      }
      break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
      if (add) {
        reflection->AddUInt64(&message, field, yaml_node.as<uint64_t>());
      } else {
        reflection->SetUInt64(&message, field, yaml_node.as<uint64_t>());
      }
      break;
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
      if (add) {
        reflection->AddFloat(&message, field, yaml_node.as<float>());
      } else {
        reflection->SetFloat(&message, field, yaml_node.as<float>());
      }
      break;
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
      if (add) {
        reflection->AddDouble(&message, field, yaml_node.as<double>());
      } else {
        reflection->SetDouble(&message, field, yaml_node.as<double>());
      }
      break;
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
      if (add) {
        reflection->AddString(&message, field, yaml_node.as<std::string>());
      } else {
        reflection->SetString(&message, field, yaml_node.as<std::string>());
      }
      break;
    default:
      break;
  }

  return 0;
}

}  // namespace yaml
}  // namespace kingfisher
