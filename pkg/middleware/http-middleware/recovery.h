#ifndef KINGFISHER_PKG_MIDDLEWARE_HTTP_MIDDLEWARE_RECOVERY_H_
#define KINGFISHER_PKG_MIDDLEWARE_HTTP_MIDDLEWARE_RECOVERY_H_

#include <exception>
#include <functional>
#include <string>

#include "cpp-httplib/httplib.h"
#include "log/config.h"

namespace kingfisher {
namespace middleware {
namespace http {

// Recovery 中间件：捕获 handler 中的异常，防止服务崩溃
// 对标 golang 的 http-middleware/debug/recoverer.go
inline void Recovery(const httplib::Request& req, httplib::Response& resp,
                     std::function<void()> next) {
  try {
    next();
  } catch (const std::exception& e) {
    LOG(ERROR) << "HTTP handler panic recovered: " << e.what()
               << ", method: " << req.method << ", path: " << req.path;
    resp.status = 500;
    resp.set_content(
        R"({"error":"internal server error","message":")" +
            std::string(e.what()) + "\"}",
        "application/json");
  } catch (...) {
    LOG(ERROR) << "HTTP handler unknown panic recovered"
               << ", method: " << req.method << ", path: " << req.path;
    resp.status = 500;
    resp.set_content(R"({"error":"internal server error"})",
                     "application/json");
  }
}

}  // namespace http
}  // namespace middleware
}  // namespace kingfisher

#endif
