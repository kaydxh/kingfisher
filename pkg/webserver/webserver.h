#ifndef KINGFISHER_PKG_WEB_SERVER_H_
#define KINGFISHER_PKG_WEB_SERVER_H_

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifdef ENABLE_GRPC
#include <grpcpp/grpcpp.h>
#endif

#include "cpp-httplib/httplib.h"
#include "google/protobuf/service.h"

namespace kingfisher {
namespace web {

// WebHandler 接口：对标 golang 项目中的 WebHandler interface
// 用户实现此接口来注册自己的 HTTP 路由和 gRPC service
class WebHandler {
 public:
  virtual ~WebHandler() = default;

  // 注册 HTTP 路由
  virtual void SetRoutes(httplib::Server& http_server) {}

#ifdef ENABLE_GRPC
  // 注册 gRPC service
  virtual void RegisterGRPCService(grpc::ServerBuilder& builder) {}
#endif
};

// PostStartHookFunc: 服务启动后的回调
using PostStartHookFunc = std::function<int()>;

// PreShutdownHookFunc: 服务关闭前的回调
using PreShutdownHookFunc = std::function<int()>;

// HealthChecker 接口：对标 golang healthz.HealthChecker
class HealthChecker {
 public:
  virtual ~HealthChecker() = default;
  virtual std::string Name() const = 0;
  virtual int Check() const = 0;  // 0 = healthy, non-zero = unhealthy
  virtual std::string Error() const { return ""; }
};

// PingHealthChecker: 始终返回健康的基本检查器
class PingHealthChecker : public HealthChecker {
 public:
  std::string Name() const override { return "ping"; }
  int Check() const override { return 0; }
};

// FuncHealthChecker: 基于函数的健康检查器
class FuncHealthChecker : public HealthChecker {
 public:
  FuncHealthChecker(const std::string& name, std::function<int()> check_func)
      : name_(name), check_func_(std::move(check_func)) {}

  std::string Name() const override { return name_; }
  int Check() const override {
    if (check_func_) return check_func_();
    return 0;
  }

 private:
  std::string name_;
  std::function<int()> check_func_;
};

// HTTP 中间件类型
// next 函数调用下一个中间件或最终 handler
using HttpMiddleware =
    std::function<void(const httplib::Request&, httplib::Response&,
                       std::function<void()> next)>;

// WebServerOptions: 配置选项
struct WebServerOptions {
  int grpc_port = 0;               // gRPC 端口，0 表示不启动 gRPC
  int http_port = 0;               // HTTP 端口
  std::string host = "0.0.0.0";    // 监听地址

  int max_concurrency = 0;         // HTTP 最大并发，0 表示不限
  int idle_timeout_sec = -1;       // HTTP 空闲超时

  // gRPC 配置
  int grpc_max_receive_message_size = 0;  // 0 = 默认
  int grpc_max_send_message_size = 0;     // 0 = 默认

  // 优雅关闭
  std::chrono::milliseconds shutdown_delay_duration{0};
  std::chrono::milliseconds shutdown_timeout_duration{5000};

  // Debug
  bool enable_profiling = false;
  std::vector<std::string> disable_print_inoutput_methods;
};

// GenericWebServer: 对标 golang 项目中的 GenericWebServer
// 核心 WebServer，同时支持 gRPC + HTTP
class GenericWebServer {
 public:
  GenericWebServer();
  ~GenericWebServer();

  // 初始化
  int Init(const WebServerOptions& opts);

  // =================== Web Handler 注册 ===================
  // 注册 WebHandler（对标 golang 的 InstallWebHandlers）
  GenericWebServer& InstallWebHandlers(
      const std::vector<std::shared_ptr<WebHandler>>& handlers);
  GenericWebServer& InstallWebHandler(std::shared_ptr<WebHandler> handler);

  // =================== HTTP 中间件 ===================
  // 添加 HTTP 中间件（对标 golang 的 http middleware chain）
  GenericWebServer& Use(HttpMiddleware middleware);

  // =================== 生命周期 Hooks ===================
  // 添加启动后 Hook（对标 golang 的 AddPostStartHook）
  int AddPostStartHook(const std::string& name, PostStartHookFunc hook);
  void AddPostStartHookOrDie(const std::string& name, PostStartHookFunc hook);

  // 添加关闭前 Hook（对标 golang 的 AddPreShutdownHook）
  int AddPreShutdownHook(const std::string& name, PreShutdownHookFunc hook);
  void AddPreShutdownHookOrDie(const std::string& name,
                               PreShutdownHookFunc hook);

  // 运行 Hooks
  void RunPostStartHooks();
  int RunPreShutdownHooks();

  // =================== 健康检查 ===================
  // 添加存活检查器（对标 golang 的 AddLivezChecker）
  void AddLivezChecker(std::shared_ptr<HealthChecker> checker);

  // 添加就绪检查器（对标 golang 的 AddReadyzChecker）
  void AddReadyzChecker(std::shared_ptr<HealthChecker> checker);

  // 设置就绪状态
  void SetReady(bool ready);
  bool IsReady() const;

  // =================== 启动和关闭 ===================
  // PrepareRun: 安装默认路由等（对标 golang 的 PrepareRun）
  int PrepareRun();

  // Run: 阻塞运行（对标 golang 的 Run）
  int Run();

  // Shutdown: 优雅关闭
  int Shutdown();

  // =================== 访问器 ===================
  httplib::Server& GetHttpServer() { return http_server_; }

#ifdef ENABLE_GRPC
  grpc::Server* GetGrpcServer() { return grpc_server_.get(); }
#endif

 private:
  // 安装默认的健康检查路由
  void installHealthzRoutes();

  // 安装 profiler 路由
  void installProfilerRoutes();

  // 安装中间件链到 cpp-httplib 全局拦截器
  // 对标 golang 的 http middleware chain
  void installMiddlewareHooks();

  // 信号处理
  void onSignalStop(int sig);

  // 运行 HTTP 服务器
  int runHttpServer();

  // 运行 gRPC 服务器
  int runGrpcServer();

  // 执行中间件链
  void executeMiddlewareChain(const httplib::Request& req,
                              httplib::Response& resp,
                              std::function<void()> final_handler);

 private:
  WebServerOptions opts_;

  // HTTP 服务器（cpp-httplib）
  httplib::Server http_server_;

#ifdef ENABLE_GRPC
  // gRPC 服务器
  std::unique_ptr<grpc::Server> grpc_server_;
  grpc::ServerBuilder grpc_builder_;
#endif

  // HTTP 中间件链
  std::vector<HttpMiddleware> middlewares_;

  // 生命周期 Hooks
  std::mutex post_start_hook_lock_;
  std::map<std::string, PostStartHookFunc> post_start_hooks_;
  bool post_start_hooks_called_ = false;

  std::mutex pre_shutdown_hook_lock_;
  std::map<std::string, PreShutdownHookFunc> pre_shutdown_hooks_;
  bool pre_shutdown_hooks_called_ = false;

  // 健康检查
  std::vector<std::shared_ptr<HealthChecker>> livez_checkers_;
  std::vector<std::shared_ptr<HealthChecker>> readyz_checkers_;
  std::atomic<bool> ready_{true};

  // 运行状态
  std::atomic<bool> running_{false};
  std::thread http_thread_;
  std::thread grpc_thread_;

  // Web handlers
  std::vector<std::shared_ptr<WebHandler>> web_handlers_;
};

}  // namespace web
}  // namespace kingfisher

#endif
