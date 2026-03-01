#ifndef KINGFISHER_PKG_MIDDLEWARE_HTTP_MIDDLEWARE_REQUEST_ID_H_
#define KINGFISHER_PKG_MIDDLEWARE_HTTP_MIDDLEWARE_REQUEST_ID_H_

#include <functional>
#include <string>

#include "cpp-httplib/httplib.h"
#include "uuid/guid.h"

namespace kingfisher {
namespace middleware {
namespace http {

// RequestID 中间件：为每个 HTTP 请求生成或提取 request_id
// 对标 golang 的 http-middleware/debug/request_id.go
inline void RequestID(const httplib::Request& req, httplib::Response& resp,
                      std::function<void()> next) {
  // 从请求头中提取 X-Request-Id，如果没有则生成新的
  std::string request_id = req.get_header_value("X-Request-Id");
  if (request_id.empty()) {
    request_id = kingfisher::uuid::Guid::GuidString();
  }
  resp.set_header("X-Request-Id", request_id);
  next();
}

}  // namespace http
}  // namespace middleware
}  // namespace kingfisher

#endif
