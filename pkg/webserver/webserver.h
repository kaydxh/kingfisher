#ifndef KINGFISHER_PKG_WEB_SERVER_H_
#define KINGFISHER_PKG_WEB_SERVER_H_

#include "brpc/server.h"
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

  int Run();

  brpc::Server& GetBrpcServer() { return server_; }

 private:
  std::string port_;
  brpc::Server server_;
  brpc::ServerOptions options_;
};

}  // namespace web
}  // namespace kingfisher

#endif
