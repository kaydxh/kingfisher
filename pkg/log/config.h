#ifndef KINGFISHER_PKG_LOG_CONFIG_H_
#define KINGFISHER_PKG_LOG_CONFIG_H_

#include <glog/logging.h>
#include <yaml-cpp/yaml.h>

#include "log.pb.h"

namespace kingfisher {
namespace log {

class CompletedConfig;

struct ConfigOptions {
  YAML::Node* node = nullptr;
  std::string app = "app";
};

class Config {
 public:
  static Config& NewConfig(const ConfigOptions& opts);

  int LoadYaml();
  CompletedConfig& Complete();

 public:
  api::v1::viper::logs::LogConfig proto_;
  ConfigOptions options_;
};

class CompletedConfig {
 public:
  CompletedConfig();
  ~CompletedConfig();
  void Init(const Config* config, int completed_ret);

  int Apply();
  int Install();

 private:
  const Config* config_ = nullptr;
  int completed_ret_ = 0;
};

}  // namespace log
}  // namespace kingfisher

#endif
