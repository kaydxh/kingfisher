#ifndef KINGFISHER_PKG_LOG_CONFIG_H_
#define KINGFISHER_PKG_LOG_CONFIG_H_

#include <yaml-cpp/yaml.h>

#include "log.pb.h"

namespace kingfisher {
namespace log {

class CompletedConfig;

struct ConfigOptions {
  YAML::Node* node = nullptr;
};

class Config {
 public:
  static std::shared_ptr<Config> NewConfig(const ConfigOptions& opts);

  int LoadYaml();
  std::shared_ptr<CompletedConfig> Complete();

 private:
  api::v1::viper::logs::LogConfig proto_;
  ConfigOptions options_;
};

class CompletedConfig {
 public:
  CompletedConfig(const Config* config, int completed_ret);
  ~CompletedConfig() {}

  int Apply();
  int Install();

 private:
  const Config* config_ = nullptr;
  int completed_ret_ = 0;
};

}  // namespace log
}  // namespace kingfisher

#endif
