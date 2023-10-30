#ifndef KINGFISHER_PKG_MIDDLEWARE_API_API_GUARD_H_
#define KINGFISHER_PKG_MIDDLEWARE_API_API_GUARD_H_

#include <iostream>
#include <type_traits>

#include "in_out_printer.h"
#include "log/config.h"
#include "middleware/api/request_id.h"
#include "time/time_counter.h"

namespace kingfisher {
namespace middleware {

template <typename REQ, typename RESP>
class ApiGuard {
 public:
  ApiGuard(const REQ* req, RESP* resp) : req_(req), resp_(resp) {
    RequestID(const_cast<REQ*>(req));
    LOG(INFO) << "recv req: " << ProtoString(req_);
  }
  ~ApiGuard() {
    tc_.Tick("api");
    LOG(INFO) << tc_.String();
    LOG(INFO) << "send resp: " << kingfisher::middleware::ProtoString(resp_);
  }

 private:
  kingfisher::time::TimeCounter tc_;
  const REQ* req_;
  RESP* resp_;
};

}  // namespace middleware
}  // namespace kingfisher
   //
#define API_GUARD                                      \
  using REQ_TYPE = std::decay<decltype(*req)>::type;   \
  using RESP_TYPE = std::decay<decltype(*resp)>::type; \
  kingfisher::middleware::ApiGuard<REQ_TYPE, RESP_TYPE> api_guard(req, resp);

#endif
