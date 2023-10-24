#include "config.h"

#include <memory>
#include <sstream>

#include "config/yaml/yaml.h"
#include "controller/date/date.h"
#include "controller/healthz/healthz.h"
#include "core/singleton.hpp"
#include "log/config.h"
#include "webserver.h"

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

int Config::parseViper() {
  auto& web_proto = proto_.web();
  std::ostringstream oss;
  oss << web_proto.bind_address().host() << ":"
      << web_proto.bind_address().port();
  options_.bind_address = oss.str();

  return 0;
}

CompletedConfig& Config::Complete() {
  int completed_ret = LoadYaml();
  auto& completed_config =
      kingfisher::core::Singleton<CompletedConfig>::Instance();
  completed_config.Init(this, completed_ret);

  parseViper();
  return completed_config;
}

CompletedConfig::CompletedConfig() {}

CompletedConfig::~CompletedConfig() {}

void CompletedConfig::Init(Config* config, int completed_ret) {
  config_ = config;
  completed_ret_ = completed_ret;
}

WebServer& CompletedConfig::ApplyOrDie() {
  if (completed_ret_ != 0) {
    // return completed_ret_;
    LOG(FATAL) << "failed to init webserver on"
               << config_->options_.bind_address << ", err:" << completed_ret_;
  }

  return InstallOrDie();
}

std::vector<std::shared_ptr<::google::protobuf::Service>>
CompletedConfig::installDefaultService() {
  auto& services = config_->options_.services;

  std::shared_ptr<::google::protobuf::Service> health_service =
      std::make_shared<HealthCheckServiceImpl>();
  services.push_back(std::move(health_service));
  std::shared_ptr<::google::protobuf::Service> date_service =
      std::make_shared<DateServiceImpl>();
  services.push_back(std::move(date_service));

  auto& ws = kingfisher::core::Singleton<WebServer>::Instance();

  for (auto& s : services) {
    ws.AddServiceOrDie(s.get());
  }

  return services;
}

WebServer& CompletedConfig::InstallOrDie() {
  auto& ws = kingfisher::core::Singleton<WebServer>::Instance();

  auto& http_proto = config_->proto_.web().http();
  WebServerOptions opts;
  opts.idle_timeout_sec = -1;
  opts.max_concurrency = http_proto.max_concurrency();
  int ret = ws.Init(config_->options_.bind_address, opts);
  if (ret != 0) {
    LOG(FATAL) << "failed to init webserver on"
               << config_->options_.bind_address << ", err:" << ret;
  }

  installDefaultService();

  return ws;
}

}  // namespace web
}  // namespace kingfisher
