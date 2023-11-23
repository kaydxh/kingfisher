#ifndef KINGFISHER_PKG_WEB_SERVER_H_
#define KINGFISHER_PKG_WEB_SERVER_H_

#ifdef ENABLE_BRPC
#include "brpc/controller.h"
#include "brpc/server.h"
#endif

#include "google/protobuf/service.h"
#include "webserver/webserverbase.h"

namespace kingfisher {
namespace web {

struct WebServerOptions {
  int max_concurrency = 0;    // Default: 0 unlimited
  int idle_timeout_sec = -1;  // Default: -1 (disabled)
};

class WebServer {
 public:
  WebServer();
  ~WebServer();

  int Init(const std::string& port, const WebServerOptions& opts);

  WebServer& AddServiceOrDie(google::protobuf::Service* service);

#ifdef ENABLE_BRPC
  WebServer& AddInterceptor(const brpc::Interceptor& interceptor);
  brpc::Server& GetBrpcServer() { return server_; }
#endif

  int Run();

 private:
  void OnSignalStop(int sig);

 private:
  std::string port_;

#ifdef ENABLE_BRPC
  brpc::Server server_;
  brpc::ServerOptions options_;
#endif
};

}  // namespace web
}  // namespace kingfisher

#endif
