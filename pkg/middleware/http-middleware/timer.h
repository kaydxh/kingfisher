#ifndef KINGFISHER_PKG_MIDDLEWARE_HTTP_MIDDLEWARE_TIMER_H_
#define KINGFISHER_PKG_MIDDLEWARE_HTTP_MIDDLEWARE_TIMER_H_

#include <functional>
#include <string>

#include "cpp-httplib/httplib.h"
#include "log/config.h"
#include "time/time_counter.h"

namespace kingfisher {
namespace middleware {
namespace http {

// Timer 中间件：记录 HTTP 请求的处理耗时
// 对标 golang 的 http-middleware/timer/timer_server.interceptor.go
inline void Timer(const httplib::Request& req, httplib::Response& resp,
                  std::function<void()> next) {
  kingfisher::time::TimeCounter tc;

  next();

  std::string callee_method = req.method + " " + req.path;
  tc.Tick(callee_method);

  std::string request_id = resp.get_header_value("X-Request-Id");
  LOG(INFO) << "[" << request_id << "] http cost " << tc.String();
}

}  // namespace http
}  // namespace middleware
}  // namespace kingfisher

#endif
