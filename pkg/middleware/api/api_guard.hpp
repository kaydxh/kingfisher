#ifndef KINGFISHER_PKG_MIDDLEWARE_API_API_GUARD_H_
#define KINGFISHER_PKG_MIDDLEWARE_API_API_GUARD_H_

#include <iostream>
#include <type_traits>

#include "google/protobuf/service.h"
#include "in_out_printer.h"
#include "log/config.h"
#include "middleware/api/request_id.h"
#include "time/time_counter.h"

namespace kingfisher {
namespace middleware {

// ApiGuard: gRPC service 方法级别的守卫
// 在构造时打印请求，析构时打印响应和耗时
// 用于 gRPC service handler 中，RAII 风格
template <typename REQ, typename RESP>
class ApiGuard {
 public:
  ApiGuard(const REQ* req, RESP* resp,
           ::google::protobuf::RpcController* controller = nullptr)
      : req_(req), resp_(resp), controller_(controller) {
    RequestID(const_cast<REQ*>(req));
    LOG(INFO) << "recv req: " << ProtoString(req_);
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
  ::google::protobuf::RpcController* controller_;
};

}  // namespace middleware
}  // namespace kingfisher

// API_GUARD 宏：在 gRPC service handler 中使用
// 自动创建 ApiGuard 实例，实现请求/响应的打印和计时
#define API_GUARD                                                            \
  using REQ_TYPE = std::decay<decltype(*req)>::type;                         \
  using RESP_TYPE = std::decay<decltype(*resp)>::type;                       \
  kingfisher::middleware::ApiGuard<REQ_TYPE, RESP_TYPE> api_guard(req, resp);

#endif
