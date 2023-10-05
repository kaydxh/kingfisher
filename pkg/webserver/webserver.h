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

  WebServer& AddServiceOrDie(google::protobuf::Service* service);

  int Run();

 private:
  brpc::Server server_;
};

}  // namespace web
}  // namespace kingfisher

#endif
