#include "yaml.h"

namespace kingfisher {
namespace yaml {

YAML::Node GetYaml(const std::string& path) {
  YAML::Node data = YAML::LoadFile(path);
  return data;
}

}  // namespace yaml
}  // namespace kingfisher
