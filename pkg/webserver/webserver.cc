#include "webserver.h"

#include <csignal>
#include <sstream>

#include "log/config.h"
#include "os/signal/signal.h"
#include "time/time_counter.h"
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

  // 安装中间件链到 cpp-httplib 的全局拦截器
  // 对标 golang 的 http middleware chain，使所有路由都经过中间件链
  installMiddlewareHooks();

  return 0;
}

void GenericWebServer::installMiddlewareHooks() {
  // 对标 golang 的 http middleware chain:
  //   RequestID → Recovery → CleanPath → Timer → InOutPrinter → Handler
  //
  // cpp-httplib 提供三个全局 hook 点:
  //   - set_pre_routing_handler: 路由匹配前执行（前置逻辑）
  //   - set_post_routing_handler: handler 执行后执行（后置逻辑）
  //   - set_exception_handler: 异常恢复（Recovery）
  //
  // 我们将中间件拆分为前置和后置两部分:
  //   前置（pre_routing）: RequestID, CleanPath
  //   后置（post_routing）: Timer（打印耗时）, InOutPrinter（打印输出）
  //   异常处理: Recovery

  // 前置中间件：在路由匹配和 handler 执行前运行
  http_server_.set_pre_routing_handler(
      [this](const httplib::Request& req, httplib::Response& resp)
          -> httplib::Server::HandlerResponse {
        // 1. RequestID（对标 golang WithHttpHandlerInterceptorRequestIDOptions）
        std::string request_id = req.get_header_value("X-Request-Id");
        if (request_id.empty()) {
          request_id = kingfisher::uuid::Guid::GuidString();
        }
        resp.set_header("X-Request-Id", request_id);

        // 记录请求开始时间（存储在 response header 中，供 post_routing 使用）
        auto now = std::chrono::steady_clock::now();
        auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          now.time_since_epoch())
                          .count();
        resp.set_header("X-Start-Time-Ns", std::to_string(now_ns));

        // 继续路由匹配
        return httplib::Server::HandlerResponse::Unhandled;
      });

  // 后置中间件：在 handler 执行后运行
  auto disable_methods = opts_.disable_print_inoutput_methods;
  http_server_.set_post_routing_handler(
      [disable_methods](const httplib::Request& req, httplib::Response& resp) {
        std::string request_id = resp.get_header_value("X-Request-Id");
        std::string callee_method = req.method + " " + req.path;

        // 检查是否禁用了该路径的打印
        bool should_print = true;
        for (const auto& m : disable_methods) {
          if (req.path == m) {
            should_print = false;
            break;
          }
        }

        // Timer: 计算耗时（对标 golang WithHttpHandlerInterceptorsTimerOptions）
        std::string start_time_str = resp.get_header_value("X-Start-Time-Ns");
        if (!start_time_str.empty()) {
          auto start_ns = std::stoll(start_time_str);
          auto now = std::chrono::steady_clock::now();
          auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            now.time_since_epoch())
                            .count();
          auto cost_us = (now_ns - start_ns) / 1000;
          double cost_ms = static_cast<double>(cost_us) / 1000.0;

          // 移除内部计时 header，避免暴露给客户端
          resp.headers.erase("X-Start-Time-Ns");

          if (should_print) {
            LOG(INFO) << "[" << request_id << "] " << callee_method
                      << ", status: " << resp.status
                      << ", resp_size: " << resp.body.size()
                      << ", cost: " << cost_ms << "ms";
          }
        }
      });

  // 异常处理（对标 golang WithHttpHandlerInterceptorRecoveryOptions）
  http_server_.set_exception_handler(
      [](const httplib::Request& req, httplib::Response& resp,
         std::exception_ptr ep) {
        std::string request_id = resp.get_header_value("X-Request-Id");
        try {
          if (ep) {
            std::rethrow_exception(ep);
          }
        } catch (const std::exception& e) {
          LOG(ERROR) << "[" << request_id
                     << "] HTTP handler exception recovered: " << e.what()
                     << ", method: " << req.method << ", path: " << req.path;
          resp.status = 500;
          resp.set_content(
              R"({"error":"internal server error","message":")" +
                  std::string(e.what()) + "\"}",
              "application/json");
        } catch (...) {
          LOG(ERROR) << "[" << request_id
                     << "] HTTP handler unknown exception recovered"
                     << ", method: " << req.method << ", path: " << req.path;
          resp.status = 500;
          resp.set_content(R"({"error":"internal server error"})",
                           "application/json");
        }
      });

  LOG(INFO) << "Installed HTTP middleware hooks: "
               "pre_routing(RequestID) + "
               "post_routing(Timer+InOutPrinter) + "
               "exception_handler(Recovery)";
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
