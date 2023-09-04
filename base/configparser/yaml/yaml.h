#ifndef KINGFISHER_BASE_CONFIGPARSER_YAML_H_
#define KINGFISHER_BASE_CONFIGPARSER_YAML_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/map_field.h>
#include <google/protobuf/message.h>
#include <google/protobuf/reflection.h>
#include <yaml-cpp/yaml.h>

namespace kingfisher {
namespace yaml {
YAML::Node GetYaml(const std::string& path);
int UnmarshalProtoMessage(const YAML::Node& yaml_node,
                          google::protobuf::Message& message);

int setValue(const YAML::Node& yaml_node,
             const google::protobuf::Reflection* reflection,
             const google::protobuf::FieldDescriptor* field,
             google::protobuf::Message& message, bool add);

int setMapKey(const YAML::Node& yaml_node,
              const google::protobuf::Reflection* reflection,
              const google::protobuf::FieldDescriptor* field,
              google::protobuf::Message& message);

}  // namespace yaml
}  // namespace kingfisher

#endif

