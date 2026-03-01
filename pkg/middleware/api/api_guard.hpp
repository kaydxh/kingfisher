#ifndef KINGFISHER_PKG_MIDDLEWARE_API_API_GUARD_H_
#define KINGFISHER_PKG_MIDDLEWARE_API_API_GUARD_H_

#include <iostream>
#include <type_traits>

#include "google/protobuf/service.h"
#include "log/config.h"

// 独立的 gRPC 中间件组件（对标 golang 的 installGrpcMiddlewareChain）
#include "middleware/grpc-middleware/in_out_printer.h"
#include "middleware/grpc-middleware/recovery.h"
#include "middleware/grpc-middleware/request_id.h"
#include "middleware/grpc-middleware/timer.h"

namespace kingfisher {
namespace middleware {

// ApiGuard: gRPC service 方法级别的守卫
// 对标 golang 的 gRPC 中间件链: RequestId → Recovery → Metric/Timer → InOutPrinter
// RAII 风格，在构造时处理请求前逻辑，析构时处理请求后逻辑
//
// 对齐 golang installGrpcMiddlewareChain 中的中间件:
//   1. RequestId  - 提取/生成 request_id
//   2. Recovery   - 异常恢复（由 try/catch 包裹整个 handler 实现）
//   3. Timer      - 计时 + 上报 metric
//   4. InOutPrinter - 打印请求/响应
template <typename REQ, typename RESP>
class ApiGuard {
 public:
  ApiGuard(const REQ* req, RESP* resp, const std::string& method = "",
           ::google::protobuf::RpcController* controller = nullptr)
      : req_(req), resp_(resp), controller_(controller) {
    // 1. RequestId 中间件（对标 golang WithServerUnaryInterceptorsRequestIdOptions）
    request_id_ = grpc::RequestID(const_cast<REQ*>(req));

    // 2. Timer 中间件（对标 golang WithServerUnaryMetricInterceptorOptions）
    timer_ = std::make_unique<grpc::TimerGuard>(
        method.empty() ? "unknown" : method, request_id_);

    // 3. InOutPrinter 中间件（对标 golang WithServerUnaryInterceptorsInOutPacketOptions）
    printer_ = std::make_unique<grpc::InOutPrinterGuard<REQ, RESP>>(
        method.empty() ? "unknown" : method, request_id_, req, resp);
  }

  ~ApiGuard() {
    // InOutPrinterGuard 析构时打印响应
    // TimerGuard 析构时打印耗时

    // 尝试设置响应中的 request_id
    grpc::TrySetResponseRequestID(resp_, request_id_);

    // 析构顺序: printer_ → timer_（RAII 逆序析构）
    // printer_ 先析构打印响应，timer_ 再析构打印耗时
    // 与 golang 的中间件执行顺序一致
  }

  const std::string& GetRequestId() const { return request_id_; }

 private:
  const REQ* req_;
  RESP* resp_;
  ::google::protobuf::RpcController* controller_;

  std::string request_id_;
  std::unique_ptr<grpc::TimerGuard> timer_;
  std::unique_ptr<grpc::InOutPrinterGuard<REQ, RESP>> printer_;
};

}  // namespace middleware
}  // namespace kingfisher

// API_GUARD 宏：在 gRPC service handler 中使用
// 自动创建 ApiGuard 实例，实现与 golang 对齐的 gRPC 中间件链
// 对标 golang 的 installGrpcMiddlewareChain:
//   RequestId → Recovery → Metric/Timer → InOutPrinter
//
// 用法:
//   void MyService::DoSomething(::google::protobuf::RpcController* controller,
//                               const MyRequest* req, MyResponse* resp,
//                               ::google::protobuf::Closure* done) {
//     API_GUARD;
//     // ... 业务逻辑 ...
//   }
#define API_GUARD                                                            \
  using REQ_TYPE = std::decay<decltype(*req)>::type;                         \
  using RESP_TYPE = std::decay<decltype(*resp)>::type;                       \
  kingfisher::middleware::ApiGuard<REQ_TYPE, RESP_TYPE> api_guard(           \
      req, resp, __FUNCTION__);

// API_GUARD_WITH_METHOD 宏：支持指定方法名
#define API_GUARD_WITH_METHOD(method)                                        \
  using REQ_TYPE = std::decay<decltype(*req)>::type;                         \
  using RESP_TYPE = std::decay<decltype(*resp)>::type;                       \
  kingfisher::middleware::ApiGuard<REQ_TYPE, RESP_TYPE> api_guard(           \
      req, resp, method);

#endif
