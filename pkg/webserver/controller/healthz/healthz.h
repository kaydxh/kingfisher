#ifndef KINGFISHER_PKG_WEB_SERVER_CONTROLLER_HEALTHZ_HEALTHZ_H_
#define KINGFISHER_PKG_WEB_SERVER_CONTROLLER_HEALTHZ_HEALTHZ_H_

#ifdef ENABLE_BRPC
#include "brpc/server.h"
#endif

#include "healthz.pb.h"

namespace kingfisher {
namespace web {

// curl http://127.0.0.1:10000/HealthCheckService/HealthCheck
class HealthCheckServiceImpl : public healthz::HealthCheckService {
 public:
  void HealthCheck(::google::protobuf::RpcController* cntl_base,
                   const healthz::HealthCheckRequest* req,
                   healthz::HealthCheckResponse* resp,
                   ::google::protobuf::Closure* done) override {
    // This object helps you to call done->Run() in RAII style. If you need
    // to process the request asynchronously, pass done_guard.release().
#ifdef ENABLE_BRPC
    brpc::ClosureGuard done_guard(done);

    brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
    cntl->http_response().set_content_type("text/plain");

    butil::IOBufBuilder os;
    os << "healthz Ok\n";
    os.move_to(cntl->response_attachment());
#endif
  }
};

}  // namespace web
}  // namespace kingfisher

#endif
