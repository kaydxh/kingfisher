#ifndef KINGFISHER_PKG_MIDDLEWARE_API_API_GUARD_H_
#define KINGFISHER_PKG_MIDDLEWARE_API_API_GUARD_H_

#include <iostream>
#include <type_traits>

#ifdef ENABLE_BRPC
#include "brpc/controller.h"
#endif

#include "google/protobuf/service.h"
#include "in_out_printer.h"
#include "log/config.h"
#include "middleware/api/request_id.h"
#include "time/time_counter.h"

namespace kingfisher {
namespace middleware {

template <typename REQ, typename RESP>
class ApiGuard {
 public:
  ApiGuard(const REQ* req, RESP* resp,
           ::google::protobuf::RpcController* controller = nullptr)
      : req_(req),
        resp_(resp),
#ifdef ENABLE_BRPC
        controller_(static_cast<brpc::Controller*>(controller))
#else
        controller_(controller)
#endif
  {
    RequestID(const_cast<REQ*>(req));

    if (controller_) {
      LOG(INFO) << "recv req: " << ProtoString(req_) << ", from remote ip: "

#ifdef ENABLE_BRPC
                << butil::endpoint2str(controller_->remote_side())
                << ", method: " << controller_->method()->full_name()
#endif
          ;
    } else {
      LOG(INFO) << "recv req: " << ProtoString(req_);
    }
  }
  ~ApiGuard() {
    tc_.Tick("api");
    LOG(INFO) << "send resp: " << kingfisher::middleware::ProtoString(resp_)
              << tc_.String();
  }

 private:
  kingfisher::time::TimeCounter tc_;
  const REQ* req_;
  RESP* resp_;

#ifdef ENABLE_BRPC
  brpc::Controller* controller_;
#else
  ::google::protobuf::RpcController* controller_;
#endif
};

}  // namespace middleware
}  // namespace kingfisher
   //
#define API_GUARD                                                            \
  using REQ_TYPE = std::decay<decltype(*req)>::type;                         \
  using RESP_TYPE = std::decay<decltype(*resp)>::type;                       \
  kingfisher::middleware::ApiGuard<REQ_TYPE, RESP_TYPE> api_guard(req, resp, \
                                                                  cntl_base);

#endif
