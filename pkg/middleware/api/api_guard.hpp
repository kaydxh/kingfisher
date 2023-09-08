#ifndef KINGFISHER_PKG_MIDDLEWARE_API_API_GUARD_H_
#define KINGFISHER_PKG_MIDDLEWARE_API_API_GUARD_H_

#include <iostream>

#include "in_out_printer.h"
#include "time/time_counter.h"

namespace kingfisher {
namespace middleware {

template <typename REQ, typename RESP>
class ApiGuard {
 public:
  ApiGuard(const REQ* req, RESP* resp) : req_(req), resp_(resp) {
    std::cout << "recv req: " << ProtoString(req_) << std::endl;
  }
  ~ApiGuard() {
    tc_.Tick("api");
    std::cout << tc_.String() << std::endl;
    std::cout << "send resp: " << kingfisher::middleware::ProtoString(resp_)
              << std::endl;
  }

 private:
  kingfisher::time::TimeCounter tc_;
  REQ const* req_;
  RESP* resp_;
};

}  // namespace middleware
}  // namespace kingfisher

#endif
