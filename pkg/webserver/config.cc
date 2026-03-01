#include "config.h"

#include <memory>
#include <sstream>

#include "config/yaml/yaml.h"
#include "core/singleton.hpp"
#include "log/config.h"
#include "webserver.h"

namespace kingfisher {
namespace web {

// =================== Config ===================

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

// =================== CompletedConfig ===================

CompletedConfig::CompletedConfig() {}
CompletedConfig::~CompletedConfig() {}

void CompletedConfig::Init(Config* config, int completed_ret) {
  config_ = config;
  completed_ret_ = completed_ret;
}

WebServerOptions CompletedConfig::buildOptions() {
  WebServerOptions opts;

  auto& web_proto = config_->proto_.web();

  // 网络地址
  opts.host = web_proto.bind_address().host();
  if (opts.host.empty()) {
    opts.host = "0.0.0.0";
  }
  opts.http_port = web_proto.bind_address().port();
  if (opts.http_port == 0) {
    opts.http_port = 80;
  }

  // gRPC 端口使用 HTTP 端口 + 1（或者也可以配置为同一端口）
  // 这里简单处理：如果 grpc 配置了相关参数，就在 http_port+1 上开启 gRPC
  auto& grpc_proto = web_proto.grpc();
  if (grpc_proto.max_concurrency_unary() > 0 ||
      grpc_proto.max_concurrency_stream() > 0 ||
      grpc_proto.max_receive_message_size() > 0 ||
      grpc_proto.max_send_message_size() > 0) {
    opts.grpc_port = opts.http_port + 1;  // gRPC 默认使用 HTTP 端口 + 1
  }

  // HTTP 配置
  auto& http_proto = web_proto.http();
  opts.max_concurrency = http_proto.max_concurrency();

  // gRPC 配置
  opts.grpc_max_receive_message_size = grpc_proto.max_receive_message_size();
  opts.grpc_max_send_message_size = grpc_proto.max_send_message_size();

  // 优雅关闭配置
  if (web_proto.has_shutdown_delay_duration()) {
    opts.shutdown_delay_duration = std::chrono::milliseconds(
        web_proto.shutdown_delay_duration().seconds() * 1000 +
        web_proto.shutdown_delay_duration().nanos() / 1000000);
  }
  if (web_proto.has_shutdown_timeout_duration()) {
    opts.shutdown_timeout_duration = std::chrono::milliseconds(
        web_proto.shutdown_timeout_duration().seconds() * 1000 +
        web_proto.shutdown_timeout_duration().nanos() / 1000000);
  }

  // Debug 配置
  auto& debug_proto = web_proto.debug();
  opts.enable_profiling = debug_proto.enable_profiling();
  for (int i = 0; i < debug_proto.disable_print_inoutput_methods_size(); ++i) {
    opts.disable_print_inoutput_methods.push_back(
        debug_proto.disable_print_inoutput_methods(i));
  }

  return opts;
}

void CompletedConfig::installDefaultMiddlewares(GenericWebServer& ws) {
  // 默认的 HTTP 中间件链（RequestID、Recovery、CleanPath、Timer、InOutPrinter）
  // 已经在 GenericWebServer::installMiddlewareHooks() 中通过 cpp-httplib 的
  // set_pre_routing_handler + set_post_routing_handler + set_exception_handler 实现
  //
  // 对标 golang 的 installHttpMiddlewareChain 中间件顺序:
  //   RequestID → Recovery → CleanPath → Timer → InOutPrinter
  //
  // 这里只需要添加用户自定义的中间件（通过 ConfigOptions.http_middlewares 传入）
  for (auto& mw : config_->options_.http_middlewares) {
    ws.Use(mw);
  }
}

void CompletedConfig::installDefaultHandlers(GenericWebServer& ws) {
  // 安装用户自定义的 WebHandler
  ws.InstallWebHandlers(config_->options_.web_handlers);
}

GenericWebServer& CompletedConfig::ApplyOrDie() {
  if (completed_ret_ != 0) {
    LOG(FATAL) << "Failed to init webserver config, err: " << completed_ret_;
  }

  auto& ws = kingfisher::core::Singleton<GenericWebServer>::Instance();

  // 构建配置
  WebServerOptions opts = buildOptions();
  int ret = ws.Init(opts);
  if (ret != 0) {
    LOG(FATAL) << "Failed to init webserver, err: " << ret;
  }

  // 安装默认中间件（对标 golang 的 installHttpMiddlewareChain）
  installDefaultMiddlewares(ws);

  // 安装默认 handler
  installDefaultHandlers(ws);

  // PrepareRun（安装健康检查等默认路由）
  ret = ws.PrepareRun();
  if (ret != 0) {
    LOG(FATAL) << "Failed to prepare run webserver, err: " << ret;
  }

  LOG(INFO) << "WebServer configured, HTTP port: " << opts.http_port
            << ", gRPC port: " << opts.grpc_port
            << ", profiling: " << (opts.enable_profiling ? "on" : "off");

  return ws;
}

}  // namespace web
}  // namespace kingfisher