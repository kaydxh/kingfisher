#ifndef KINGFISHER_PKG_MIDDLEWARE_GRPC_MIDDLEWARE_RECOVERY_H_
#define KINGFISHER_PKG_MIDDLEWARE_GRPC_MIDDLEWARE_RECOVERY_H_

#include <exception>
#include <functional>
#include <string>

#include "log/config.h"

namespace kingfisher {
namespace middleware {
namespace grpc {

// gRPC Recovery 中间件：捕获 service handler 中的异常
// 对标 golang 的 grpc-gateway WithServerInterceptorsRecoveryOptions
//
// 用法：
//   auto result = grpc::RecoveryWrap([&]() { return handler(req); });
template <typename Func>
auto RecoveryWrap(const std::string& method, Func&& func)
    -> decltype(func()) {
  try {
    return func();
  } catch (const std::exception& e) {
    LOG(ERROR) << "gRPC handler panic recovered: " << e.what()
               << ", method: " << method;
    throw;  // 重新抛出，让上层处理
  } catch (...) {
    LOG(ERROR) << "gRPC handler unknown panic recovered"
               << ", method: " << method;
    throw;
  }
}

}  // namespace grpc
}  // namespace middleware
}  // namespace kingfisher

#endif
