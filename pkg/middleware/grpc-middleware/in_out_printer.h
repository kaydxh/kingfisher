#ifndef KINGFISHER_PKG_MIDDLEWARE_GRPC_MIDDLEWARE_IN_OUT_PRINTER_H_
#define KINGFISHER_PKG_MIDDLEWARE_GRPC_MIDDLEWARE_IN_OUT_PRINTER_H_

#include <string>

#include "google/protobuf/message.h"
#include "log/config.h"
#include "middleware/api/in_out_printer.h"

namespace kingfisher {
namespace middleware {
namespace grpc {

// gRPC InOutPrinter：RAII 风格的请求/响应打印器
// 对标 golang 的 grpc-middleware/debug/in_output_printer_server.interceptor.go
//
// 用法：在 gRPC handler 开头创建实例
//   grpc::InOutPrinterGuard<REQ, RESP> printer(method, request_id, req, resp);
template <typename REQ, typename RESP>
class InOutPrinterGuard {
 public:
  InOutPrinterGuard(const std::string& method, const std::string& request_id,
                    const REQ* req, RESP* resp)
      : method_(method), request_id_(request_id), resp_(resp) {
    // 打印请求（recv）
    LOG(INFO) << "[" << request_id_ << "] recv " << method_
              << ", request: " << kingfisher::middleware::ProtoString(req);
  }

  ~InOutPrinterGuard() {
    // 打印响应（send）
    LOG(INFO) << "[" << request_id_ << "] send " << method_
              << ", response: " << kingfisher::middleware::ProtoString(resp_);
  }

 private:
  std::string method_;
  std::string request_id_;
  RESP* resp_;
};

}  // namespace grpc
}  // namespace middleware
}  // namespace kingfisher

#endif
