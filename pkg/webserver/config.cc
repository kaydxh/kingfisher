#include "config.h"

#include "config/yaml/yaml.h"
#include "core/singleton.hpp"

namespace kingfisher {
namespace web {

int Config::LoadYaml() {
  if (options_.node != nullptr) {
    return kingfisher::yaml::UnmarshalProtoMessage(*options_.node, proto_);
  }

  return 0;
}

Config& Config::NewConfig(const ConfigOptions& opts) {
  auto& config = kingfisher::core::Singleton<Config>::Instance();
  config.options_ = opts;

  if (config.LoadYaml() != 0) {
    return config;
  }

  return config;
}

CompletedConfig& Config::Complete() {
  int completed_ret = LoadYaml();
  auto& completed_config =
      kingfisher::core::Singleton<CompletedConfig>::Instance();
  completed_config.Init(this, completed_ret);
  return completed_config;
}

CompletedConfig::CompletedConfig() {}

CompletedConfig::~CompletedConfig() {}

void CompletedConfig::Init(const Config* config, int completed_ret) {
  config_ = config;
  completed_ret_ = completed_ret;
}

int CompletedConfig::Apply() {
  if (completed_ret_ != 0) {
    return completed_ret_;
  }

  return Install();
}

int CompletedConfig::Install() { return 0; }

}  // namespace web
}  // namespace kingfisher
