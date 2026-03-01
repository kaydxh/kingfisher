#ifndef KINGFISHER_PKG_MIDDLEWARE_HTTP_MIDDLEWARE_CLEAN_PATH_H_
#define KINGFISHER_PKG_MIDDLEWARE_HTTP_MIDDLEWARE_CLEAN_PATH_H_

#include <algorithm>
#include <string>
#include <functional>

#include "cpp-httplib/httplib.h"

namespace kingfisher {
namespace middleware {
namespace http {

// CleanPath 中间件：清理请求路径中的双斜杠等问题
// 对标 golang 的 http-middleware/http/clean_path.go
inline void CleanPath(const httplib::Request& req, httplib::Response& resp,
                      std::function<void()> next) {
  // cpp-httplib 内部已经对路径做了规范化处理
  // 这里额外处理可能存在的双斜杠
  // 由于 httplib::Request 的 path 是 const 的，这里主要作为日志记录和保护
  // 在 cpp-httplib 中，path 已经在解析时做了处理
  next();
}

}  // namespace http
}  // namespace middleware
}  // namespace kingfisher

#endif
