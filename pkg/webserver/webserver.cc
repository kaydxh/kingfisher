#include "webserver.h"

#include <csignal>
#include <sstream>

#include "log/config.h"
#include "os/signal/signal.h"
#include "time/timestamp.h"
#include "uuid/guid.h"

namespace kingfisher {
namespace web {

// =================== 构造/析构 ===================

GenericWebServer::GenericWebServer() {
  // 默认添加 ping 健康检查器
  livez_checkers_.push_back(std::make_shared<PingHealthChecker>());
  readyz_checkers_.push_back(std::make_shared<PingHealthChecker>());
}

GenericWebServer::~GenericWebServer() {
  if (running_.load()) {
    Shutdown();
  }
}

// =================== 初始化 ===================

int GenericWebServer::Init(const WebServerOptions& opts) {
  opts_ = opts;
  return 0;
}

// =================== Web Handler 注册 ===================

GenericWebServer& GenericWebServer::InstallWebHandlers(
    const std::vector<std::shared_ptr<WebHandler>>& handlers) {
  for (auto& h : handlers) {
    if (h) {
      InstallWebHandler(h);
    }
  }
  return *this;
}

GenericWebServer& GenericWebServer::InstallWebHandler(
    std::shared_ptr<WebHandler> handler) {
  if (handler) {
    web_handlers_.push_back(handler);
    // 注册 HTTP 路由
    handler->SetRoutes(http_server_);
#ifdef ENABLE_GRPC
    // 注册 gRPC service
    handler->RegisterGRPCService(grpc_builder_);
#endif
  }
  return *this;
}

// =================== HTTP 中间件 ===================

GenericWebServer& GenericWebServer::Use(HttpMiddleware middleware) {
  middlewares_.push_back(std::move(middleware));
  return *this;
}

void GenericWebServer::executeMiddlewareChain(
    const httplib::Request& req, httplib::Response& resp,
    std::function<void()> final_handler) {
  if (middlewares_.empty()) {
    final_handler();
    return;
  }

  // 递归构建中间件链
  std::function<void(size_t)> chain = [&](size_t index) {
    if (index >= middlewares_.size()) {
      final_handler();
      return;
    }
    middlewares_[index](req, resp, [&chain, index]() { chain(index + 1); });
  };

  chain(0);
}

// =================== 生命周期 Hooks ===================

int GenericWebServer::AddPostStartHook(const std::string& name,
                                       PostStartHookFunc hook) {
  if (name.empty()) {
    LOG(ERROR) << "AddPostStartHook: missing name";
    return -1;
  }
  if (!hook) {
    LOG(ERROR) << "AddPostStartHook: hook func may not be null: " << name;
    return -1;
  }

  std::lock_guard<std::mutex> lock(post_start_hook_lock_);
  if (post_start_hooks_called_) {
    LOG(ERROR) << "AddPostStartHook: unable to add " << name
               << " because PostStartHooks have already been called";
    return -1;
  }
  if (post_start_hooks_.count(name) > 0) {
    LOG(ERROR) << "AddPostStartHook: unable to add " << name
               << " because it was already registered";
    return -1;
  }

  post_start_hooks_[name] = std::move(hook);
  return 0;
}

void GenericWebServer::AddPostStartHookOrDie(const std::string& name,
                                             PostStartHookFunc hook) {
  if (AddPostStartHook(name, std::move(hook)) != 0) {
    LOG(FATAL) << "Error registering PostStartHook " << name;
  }
}

int GenericWebServer::AddPreShutdownHook(const std::string& name,
                                         PreShutdownHookFunc hook) {
  if (name.empty()) {
    LOG(ERROR) << "AddPreShutdownHook: missing name";
    return -1;
  }
  if (!hook) {
    return 0;
  }

  std::lock_guard<std::mutex> lock(pre_shutdown_hook_lock_);
  if (pre_shutdown_hooks_called_) {
    LOG(ERROR) << "AddPreShutdownHook: unable to add " << name
               << " because PreShutdownHooks have already been called";
    return -1;
  }
  if (pre_shutdown_hooks_.count(name) > 0) {
    LOG(ERROR) << "AddPreShutdownHook: unable to add " << name
               << " because it is already registered";
    return -1;
  }

  pre_shutdown_hooks_[name] = std::move(hook);
  return 0;
}

void GenericWebServer::AddPreShutdownHookOrDie(const std::string& name,
                                               PreShutdownHookFunc hook) {
  if (AddPreShutdownHook(name, std::move(hook)) != 0) {
    LOG(FATAL) << "Error registering PreShutdownHook " << name;
  }
}

void GenericWebServer::RunPostStartHooks() {
  std::lock_guard<std::mutex> lock(post_start_hook_lock_);
  post_start_hooks_called_ = true;

  for (auto& [name, hook] : post_start_hooks_) {
    // 在独立线程中运行每个 hook（对标 golang 的 go routine）
    std::thread([name, hook]() {
      LOG(INFO) << "Running PostStartHook: " << name;
      int ret = hook();
      if (ret != 0) {
        LOG(FATAL) << "PostStartHook " << name << " failed, ret: " << ret;
      }
      LOG(INFO) << "PostStartHook " << name << " finished";
    }).detach();
  }
}

int GenericWebServer::RunPreShutdownHooks() {
  std::lock_guard<std::mutex> lock(pre_shutdown_hook_lock_);
  pre_shutdown_hooks_called_ = true;

  int result = 0;
  for (auto& [name, hook] : pre_shutdown_hooks_) {
    LOG(INFO) << "Running PreShutdownHook: " << name;
    int ret = hook();
    if (ret != 0) {
      LOG(ERROR) << "PreShutdownHook " << name << " failed, ret: " << ret;
      result = ret;
    }
  }
  return result;
}

// =================== 健康检查 ===================

void GenericWebServer::AddLivezChecker(
    std::shared_ptr<HealthChecker> checker) {
  livez_checkers_.push_back(std::move(checker));
}

void GenericWebServer::AddReadyzChecker(
    std::shared_ptr<HealthChecker> checker) {
  readyz_checkers_.push_back(std::move(checker));
}

void GenericWebServer::SetReady(bool ready) { ready_.store(ready); }

bool GenericWebServer::IsReady() const { return ready_.load(); }

void GenericWebServer::installHealthzRoutes() {
  // /healthz - 综合健康检查（对标 golang 的 /healthz）
  http_server_.Get("/healthz", [this](const httplib::Request& req,
                                      httplib::Response& resp) {
    // 检查存活
    for (auto& checker : livez_checkers_) {
      if (checker->Check() != 0) {
        resp.status = 503;
        resp.set_content(
            R"({"status":"unhealthy","type":"livez","error":")" +
                checker->Name() + " failed\"}",
            "application/json");
        return;
      }
    }
    // 检查就绪
    if (!ready_.load()) {
      resp.status = 503;
      resp.set_content(
          R"({"status":"not ready","type":"readyz","error":"server is shutting down"})",
          "application/json");
      return;
    }
    for (auto& checker : readyz_checkers_) {
      if (checker->Check() != 0) {
        resp.status = 503;
        resp.set_content(
            R"({"status":"not ready","type":"readyz","error":")" +
                checker->Name() + " failed\"}",
            "application/json");
        return;
      }
    }
    resp.status = 200;
    resp.set_content("ok", "text/plain");
  });

  // /livez - 存活检查（对标 K8s liveness probe）
  http_server_.Get("/livez", [this](const httplib::Request& req,
                                    httplib::Response& resp) {
    for (auto& checker : livez_checkers_) {
      if (checker->Check() != 0) {
        resp.status = 503;
        resp.set_content(
            R"({"status":"unhealthy","error":")" + checker->Name() +
                " failed\"}",
            "application/json");
        return;
      }
    }
    resp.status = 200;
    resp.set_content("ok", "text/plain");
  });

  // /readyz - 就绪检查（对标 K8s readiness probe）
  http_server_.Get("/readyz", [this](const httplib::Request& req,
                                     httplib::Response& resp) {
    if (!ready_.load()) {
      resp.status = 503;
      resp.set_content(
          R"({"status":"not ready","error":"server is shutting down"})",
          "application/json");
      return;
    }
    for (auto& checker : readyz_checkers_) {
      if (checker->Check() != 0) {
        resp.status = 503;
        resp.set_content(
            R"({"status":"not ready","error":")" + checker->Name() +
                " failed\"}",
            "application/json");
        return;
      }
    }
    resp.status = 200;
    resp.set_content("ok", "text/plain");
  });

  // /healthz/verbose - 详细健康检查
  http_server_.Get("/healthz/verbose", [this](const httplib::Request& req,
                                              httplib::Response& resp) {
    std::ostringstream oss;
    oss << R"({"status":)";
    bool all_healthy = true;

    oss << R"("checks":[)";
    bool first = true;

    // 存活检查
    for (auto& checker : livez_checkers_) {
      if (!first) oss << ",";
      first = false;
      int ret = checker->Check();
      bool healthy = (ret == 0);
      if (!healthy) all_healthy = false;
      oss << R"({"name":")" << checker->Name() << R"(","healthy":)"
          << (healthy ? "true" : "false") << "}";
    }

    // 就绪检查
    if (!ready_.load()) {
      all_healthy = false;
    }
    for (auto& checker : readyz_checkers_) {
      if (!first) oss << ",";
      first = false;
      int ret = checker->Check();
      bool healthy = (ret == 0);
      if (!healthy) all_healthy = false;
      oss << R"({"name":")" << checker->Name() << R"(","healthy":)"
          << (healthy ? "true" : "false") << "}";
    }

    oss << "],";
    oss << R"("status":")" << (all_healthy ? "healthy" : "unhealthy")
        << R"("})";

    resp.status = all_healthy ? 200 : 503;
    resp.set_content(oss.str(), "application/json");
  });

  LOG(INFO) << "Installed healthz routes: /healthz, /livez, /readyz, "
               "/healthz/verbose";
}

// =================== Profiler 路由 ===================

void GenericWebServer::installProfilerRoutes() {
  if (!opts_.enable_profiling) {
    return;
  }

  // /debug/pprof/heap - 内存 profile（使用 tcmalloc）
  http_server_.Get("/debug/pprof/heap", [](const httplib::Request& req,
                                           httplib::Response& resp) {
    resp.status = 200;
    resp.set_content(
        R"({"message":"heap profiling endpoint - use gperftools pprof to analyze"})",
        "application/json");
  });

  // /debug/pprof/profile - CPU profile
  http_server_.Get("/debug/pprof/profile", [](const httplib::Request& req,
                                              httplib::Response& resp) {
    resp.status = 200;
    resp.set_content(
        R"({"message":"cpu profiling endpoint - use gperftools pprof to analyze"})",
        "application/json");
  });

  LOG(INFO) << "Installed profiler routes: /debug/pprof/*";
}

// =================== 信号处理 ===================

void GenericWebServer::onSignalStop(int sig) {
  LOG(INFO) << "Received signal " << sig << ", initiating shutdown...";
  Shutdown();
}

// =================== HTTP 服务器 ===================

int GenericWebServer::runHttpServer() {
  if (opts_.http_port <= 0) {
    LOG(ERROR) << "Invalid HTTP port: " << opts_.http_port;
    return -1;
  }

  std::ostringstream oss;
  oss << opts_.host << ":" << opts_.http_port;
  std::string addr = oss.str();

  LOG(INFO) << "Starting HTTP server, bindling on " << addr << " ...";

  // cpp-httplib 的 listen() 会阻塞在事件循环中，直到 stop() 被调用
  // 如果绑定端口失败会返回 false 并立即退出
  bool ret = http_server_.listen(opts_.host, opts_.http_port);
  if (!ret) {
    LOG(ERROR) << "HTTP server exited or failed to bind on " << addr
               << ", please check if the port is already in use";
    return -1;
  }

  LOG(INFO) << "HTTP server stopped on " << addr;
  return 0;
}

// =================== gRPC 服务器 ===================

int GenericWebServer::runGrpcServer() {
#ifdef ENABLE_GRPC
  std::ostringstream oss;
  oss << opts_.host << ":" << opts_.grpc_port;
  std::string addr = oss.str();

  grpc_builder_.AddListeningPort(addr, grpc::InsecureServerCredentials());

  // 设置消息大小限制
  if (opts_.grpc_max_receive_message_size > 0) {
    grpc_builder_.SetMaxReceiveMessageSize(
        opts_.grpc_max_receive_message_size);
  }
  if (opts_.grpc_max_send_message_size > 0) {
    grpc_builder_.SetMaxSendMessageSize(opts_.grpc_max_send_message_size);
  }

  grpc_server_ = grpc_builder_.BuildAndStart();
  if (!grpc_server_) {
    LOG(ERROR) << "Failed to start gRPC server on " << addr;
    return -1;
  }

  LOG(INFO) << "gRPC server started on " << addr;
  grpc_server_->Wait();
  LOG(INFO) << "gRPC server stopped on " << addr;
#endif
  return 0;
}

// =================== PrepareRun ===================

int GenericWebServer::PrepareRun() {
  // 安装默认路由
  installHealthzRoutes();
  installProfilerRoutes();

  return 0;
}

// =================== Run ===================

int GenericWebServer::Run() {
  running_.store(true);

  // 安装信号处理
  os::SignalHandler::InstallStopHandler(
      std::bind(&GenericWebServer::onSignalStop, this, std::placeholders::_1));
  os::SignalHandler::SetCoreDump(true, -1);

  std::ostringstream http_addr;
  http_addr << opts_.host << ":" << opts_.http_port;
  LOG(INFO) << "Starting web server, HTTP on " << http_addr.str();

#ifdef ENABLE_GRPC
  // 如果配置了 gRPC 端口，在单独的线程中启动 gRPC 服务器
  if (opts_.grpc_port > 0) {
    grpc_thread_ = std::thread([this]() { runGrpcServer(); });
    std::ostringstream grpc_addr;
    grpc_addr << opts_.host << ":" << opts_.grpc_port;
    LOG(INFO) << "gRPC server thread started on " << grpc_addr.str();
  }
#endif

  // 运行启动后 hooks
  RunPostStartHooks();

  LOG(INFO) << "Web server is ready, HTTP listening on " << http_addr.str()
            << " (blocking until shutdown)";

  // 在主线程中运行 HTTP 服务器（阻塞，直到 stop() 被调用）
  int ret = runHttpServer();

  // HTTP 服务器退出后的清理逻辑
  // 标记服务不再就绪
  SetReady(false);

  // 等待关闭延迟，让负载均衡摘除节点
  if (opts_.shutdown_delay_duration.count() > 0) {
    LOG(INFO) << "Waiting "
              << opts_.shutdown_delay_duration.count()
              << "ms before shutdown for connection draining";
    std::this_thread::sleep_for(opts_.shutdown_delay_duration);
  }

  // 运行关闭前 hooks
  int hook_ret = RunPreShutdownHooks();
  if (hook_ret != 0) {
    LOG(ERROR) << "Failed to run pre-shutdown hooks, ret: " << hook_ret;
  }

#ifdef ENABLE_GRPC
  // 关闭 gRPC 服务器
  if (grpc_server_) {
    auto deadline = std::chrono::system_clock::now() +
                    opts_.shutdown_timeout_duration;
    grpc_server_->Shutdown(deadline);
    LOG(INFO) << "gRPC server shut down";
  }

  // 等待 gRPC 线程
  if (grpc_thread_.joinable()) {
    grpc_thread_.join();
  }
#endif

  running_.store(false);
  LOG(INFO) << "Web server shut down";

  return ret;
}

// =================== Shutdown ===================

int GenericWebServer::Shutdown() {
  LOG(INFO) << "Shutting down web server...";

  // 标记不再就绪
  SetReady(false);

  // 停止 HTTP 服务器
  http_server_.stop();

#ifdef ENABLE_GRPC
  // 停止 gRPC 服务器
  if (grpc_server_) {
    auto deadline = std::chrono::system_clock::now() +
                    opts_.shutdown_timeout_duration;
    grpc_server_->Shutdown(deadline);
  }
#endif

  return 0;
}

}  // namespace web
}  // namespace kingfisher
