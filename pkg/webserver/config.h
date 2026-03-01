#ifndef KINGFISHER_PKG_WEB_SERVER_CONFIG_H_
#define KINGFISHER_PKG_WEB_SERVER_CONFIG_H_

#include <memory>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "webserver.h"
#include "webserver.pb.h"

namespace kingfisher {
namespace web {

class CompletedConfig;

// ConfigOptions: 创建 WebServer 的配置选项
struct ConfigOptions {
  YAML::Node* node = nullptr;
  std::string bind_address;
  std::string external_address;

  // 用户自定义的 WebHandler
  std::vector<std::shared_ptr<WebHandler>> web_handlers;

  // 额外的 gRPC gateway 选项
  std::vector<HttpMiddleware> http_middlewares;
};

// Config: 配置管理（对标 golang 的 Config）
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

// CompletedConfig: 完成后的配置（对标 golang 的 CompletedConfig）
class CompletedConfig {
 public:
  CompletedConfig();
  ~CompletedConfig();

  void Init(Config* config, int completed_ret);

  // ApplyOrDie: 创建并初始化 WebServer（对标 golang 的 New）
  GenericWebServer& ApplyOrDie();

 private:
  // 安装默认的 HTTP 中间件链（对标 golang 的 installHttpMiddlewareChain）
  void installDefaultMiddlewares(GenericWebServer& ws);

  // 安装默认的 WebHandler（健康检查已内置）
  void installDefaultHandlers(GenericWebServer& ws);

  // 从 proto 配置构建 WebServerOptions
  WebServerOptions buildOptions();

 private:
  Config* config_ = nullptr;
  int completed_ret_ = 0;
};

}  // namespace web
}  // namespace kingfisher

#endif
