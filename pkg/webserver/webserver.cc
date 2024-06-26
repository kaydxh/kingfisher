#include "webserver.h"

#include "log/config.h"
#include "os/signal/signal.h"

namespace kingfisher {
namespace web {

WebServer::WebServer() {}
WebServer::~WebServer() {}

int WebServer::Init(const std::string& port, const WebServerOptions& opts) {
  port_ = port;
#ifdef ENABLE_BRPC
  options_.idle_timeout_sec = opts.idle_timeout_sec;
  options_.max_concurrency = opts.max_concurrency;
#endif

  return 0;
}

WebServer& WebServer::AddServiceOrDie(google::protobuf::Service* service) {
#ifdef ENABLE_BRPC
  int ret = server_.AddService(service, brpc::SERVER_DOESNT_OWN_SERVICE);
  if (ret != 0) {
    LOG(FATAL) << "failed to add service, ret: " << ret;
  }
#endif

  return *this;
}

#ifdef ENABLE_BRPC
WebServer& WebServer::AddInterceptor(const brpc::Interceptor& interceptor) {
  //  options_.interceptors.push_back(interceptor);
  // server_.AddInterceptor(&interceptor);
  options_.interceptor = &interceptor;
  return *this;
}
#endif

int WebServer::Run() {
  LOG(INFO) << "install server on " << port_;

  os::SignalHandler::InstallStopHandler(
      std::bind(&WebServer::OnSignalStop, this, std::placeholders::_1));
  os::SignalHandler::SetCoreDump(true, -1);

#ifdef ENABLE_BRPC
  int ret = server_.Start(port_.c_str(), &options_);
  if (ret != 0) {
    LOG(ERROR) << "failec to run server on " << port_ << ", ret: " << ret;
    return ret;
  }

  server_.RunUntilAskedToQuit();
#endif
  return 0;
}

void WebServer::OnSignalStop(int sig) {}

}  // namespace web
}  // namespace kingfisher
