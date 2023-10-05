#include "webserver.h"

namespace kingfisher {
namespace web {

WebServer::WebServer() {}
WebServer::~WebServer() {}

WebServer& WebServer::AddServiceOrDie(google::protobuf::Service* service) {
  int ret = server_.AddService(service, brpc::SERVER_DOESNT_OWN_SERVICE);
  if (ret != 0) {
    LOG(FATAL) << "failed to add service, ret: " << ret;
  }

  return *this;
}

}  // namespace web
}  // namespace kingfisher
