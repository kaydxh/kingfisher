#ifndef KINGFISHER_PKG_MIDDLEWARE_GRPC_MIDDLEWARE_TIMER_H_
#define KINGFISHER_PKG_MIDDLEWARE_GRPC_MIDDLEWARE_TIMER_H_

#include <string>

#include "log/config.h"
#include "time/time_counter.h"

namespace kingfisher {
namespace middleware {
namespace grpc {

// gRPC Timer：RAII 风格的请求计时器
// 对标 golang 的 grpc-middleware/timer/timer_server.interceptor.go
// 以及 grpc-middleware/opentelemetry/metric_server.interceptor.go
//
// 用法：在 gRPC handler 开头创建 TimerGuard 实例
//   grpc::TimerGuard timer("method_name", request_id);
class TimerGuard {
 public:
  TimerGuard(const std::string& method, const std::string& request_id = "")
      : method_(method), request_id_(request_id) {}

  ~TimerGuard() {
    tc_.Tick(method_);
    if (!request_id_.empty()) {
      LOG(INFO) << "[" << request_id_ << "] grpc cost " << tc_.String();
    } else {
      LOG(INFO) << "grpc cost " << tc_.String();
    }
  }

 private:
  kingfisher::time::TimeCounter tc_;
  std::string method_;
  std::string request_id_;
};

}  // namespace grpc
}  // namespace middleware
}  // namespace kingfisher

#endif
