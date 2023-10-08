#ifndef KINGFISHER_PKG_WEB_SERVER_H_
#define KINGFISHER_PKG_WEB_SERVER_H_

#include "brpc/server.h"
#include "webserver/webserverbase.h"

namespace kingfisher {
namespace web {

struct WebServerOptions {};

class WebServer {
 public:
  WebServer();
  ~WebServer();

  int Init(const std::string& port);

  WebServer& AddServiceOrDie(google::protobuf::Service* service);
  int Run();

 private:
  std::string port_;
  brpc::Server server_;
  brpc::ServerOptions options_;
};

}  // namespace web
}  // namespace kingfisher

#endif
