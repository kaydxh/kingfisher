#ifndef KINGFISHER_PKG_WEB_SERVER_CONFIG_H_
#define KINGFISHER_PKG_WEB_SERVER_CONFIG_H_

#include <google/protobuf/service.h>
#include <yaml-cpp/yaml.h>

#include "webserver.pb.h"

namespace kingfisher {
namespace web {

class CompletedConfig;
class WebServer;

struct ConfigOptions {
  YAML::Node* node = nullptr;
  std::string bind_address;
  std::string external_address;

  std::vector<std::shared_ptr<::google::protobuf::Service>> services;
};

class Config {
 public:
  static Config& NewConfig(const ConfigOptions& opts);

  int LoadYaml();
  CompletedConfig& Complete();

 public:
  go::pkg::webserver::WebConfig proto_;
  ConfigOptions options_;

 private:
  int parseViper();
};

class CompletedConfig {
 public:
  CompletedConfig();
  ~CompletedConfig();
  void Init(Config* config, int completed_ret);

  WebServer& ApplyOrDie();
  WebServer& InstallOrDie();

 private:
  std::vector<std::shared_ptr<::google::protobuf::Service>>
  installDefaultService();

 private:
  Config* config_ = nullptr;
  int completed_ret_ = 0;
};

}  // namespace web
}  // namespace kingfisher

#endif
