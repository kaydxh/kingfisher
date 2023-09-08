#include "config.h"

#include <memory>

#include "config/yaml/yaml.h"

namespace kingfisher {
namespace log {

int Config::LoadYaml() {
  if (options.node != nullptr) {
    return kingfisher::yaml::UnmarshalProtoMessage(*options.node, proto);
  }

  return 0;
}

std::shared_ptr<Config> Config::NewConfig(const ConfigOptions& opts) {
  std::shared_ptr<Config> config = std::make_shared<Config>();
  config->options = opts;

  if (config->LoadYaml() != 0) {
    return nullptr;
  }

  return config;
}

std::shared_ptr<CompletedConfig> Config::Complete() {
  int completed_ret = LoadYaml();
  std::cout << "log:" << proto.DebugString() << std::endl;
  std::shared_ptr<CompletedConfig> completed_config =
      std::make_shared<CompletedConfig>(this, completed_ret);
  return completed_config;
}

CompletedConfig::CompletedConfig(const Config* config, int completed_ret)
    : config_(const_cast<Config*>(config)), completed_ret_(completed_ret) {}

int CompletedConfig::Apply() {
  if (completed_ret_ != 0) {
    return completed_ret_;
  }

  return Install();
}

int CompletedConfig::Install() { return 0; }

}  // namespace log
}  // namespace kingfisher
