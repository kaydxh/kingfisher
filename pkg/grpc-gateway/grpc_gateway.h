#ifndef KINGFISHER_PKG_GRPC_GATEWAY_GRPC_GATEWAY_H_
#define KINGFISHER_PKG_GRPC_GATEWAY_GRPC_GATEWAY_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "cpp-httplib/httplib.h"

#ifdef ENABLE_GRPC
#include <grpcpp/grpcpp.h>
#endif

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

namespace kingfisher {
namespace grpcgateway {

// GRPCRoute: 定义一个 HTTP → gRPC 的路由映射
// 对标 golang grpc-gateway 中 proto 注解生成的路由
struct GRPCRoute {
  std::string http_method;  // GET, POST, PUT, DELETE
  std::string http_path;    // /api/v1/users/{user_id}

  // HTTP 请求处理函数
  // 在函数内部完成: HTTP JSON → Protobuf → 调用 gRPC stub → Protobuf → HTTP JSON
  using Handler = std::function<void(const httplib::Request& req,
                                     httplib::Response& resp)>;
  Handler handler;
};

// GRPCGateway: HTTP → gRPC 代理网关
// 对标 golang 的 grpc-gateway，将 HTTP/JSON 请求转换为 gRPC 调用
//
// 架构:
//   Client → HTTP/JSON → GRPCGateway → gRPC stub (in-process) → gRPC Server
//                                    ← gRPC response ← Protobuf → JSON → Client
class GRPCGateway {
 public:
  GRPCGateway();
  ~GRPCGateway();

#ifdef ENABLE_GRPC
  // 设置 gRPC channel（用于 in-process 调用）
  void SetChannel(std::shared_ptr<grpc::Channel> channel);

  // 获取 gRPC channel
  std::shared_ptr<grpc::Channel> GetChannel() const { return channel_; }

  // 创建到本地 gRPC server 的 channel
  static std::shared_ptr<grpc::Channel> CreateInProcessChannel(
      const std::string& target);
#endif

  // 注册 HTTP → gRPC 路由映射
  void RegisterRoute(const GRPCRoute& route);

  // 将所有注册的路由安装到 HTTP server
  void InstallRoutes(httplib::Server& http_server);

  // 工具方法：Protobuf message → JSON string
  static std::string ProtoToJson(const google::protobuf::Message& message);

  // 工具方法：JSON string → Protobuf message
  static int JsonToProto(const std::string& json,
                         google::protobuf::Message* message);

 private:
#ifdef ENABLE_GRPC
  std::shared_ptr<grpc::Channel> channel_;
#endif
  std::vector<GRPCRoute> routes_;
};

}  // namespace grpcgateway
}  // namespace kingfisher

#endif
