#ifndef KINGFISHER_BASE_CONFIGPARSER_YAML_H_
#define KINGFISHER_BASE_CONFIGPARSER_YAML_H_

#include <yaml-cpp/yaml.h>

namespace kingfisher {
namespace yaml {
YAML::Node GetYaml(const std::string& path);

}
}  // namespace kingfisher

#endif

