#ifndef KINGFISHER_PKG_MIDDLEWARE_HTTP_MIDDLEWARE_IN_OUT_PRINTER_H_
#define KINGFISHER_PKG_MIDDLEWARE_HTTP_MIDDLEWARE_IN_OUT_PRINTER_H_

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include "cpp-httplib/httplib.h"
#include "log/config.h"

namespace kingfisher {
namespace middleware {
namespace http {

// InOutPrinterFactory 创建一个 InOutPrinter 中间件
// 支持通过 disable_methods 禁用特定路径的打印
// 对标 golang 的 http-middleware/debug/in.output_printer.go
inline std::function<void(const httplib::Request&, httplib::Response&,
                          std::function<void()>)>
MakeInOutPrinter(const std::vector<std::string>& disable_methods = {}) {
  return [disable_methods](const httplib::Request& req,
                           httplib::Response& resp,
                           std::function<void()> next) {
    // 检查是否禁用了该路径的打印
    bool should_print = true;
    for (const auto& m : disable_methods) {
      if (req.path == m) {
        should_print = false;
        break;
      }
    }

    std::string request_id = resp.get_header_value("X-Request-Id");
    std::string callee_method = req.method + " " + req.path;

    if (should_print) {
      LOG(INFO) << "[" << request_id << "] recv " << callee_method
                << ", body_size: " << req.body.size();
    }

    next();

    if (should_print) {
      LOG(INFO) << "[" << request_id << "] send " << callee_method
                << ", status: " << resp.status
                << ", resp_size: " << resp.body.size();
    }
  };
}

}  // namespace http
}  // namespace middleware
}  // namespace kingfisher

#endif
