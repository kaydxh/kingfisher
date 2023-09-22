#ifndef KINGFISHER_PKG_GRPC_GATEWAY_GRPC_GATEWAY_H_
#define KINGFISHER_PKG_GRPC_GATEWAY_GRPC_GATEWAY_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

#include "cpp-httplib/httplib.h"

namespace kingfisher {
namespace grpcgateway {

class GRPCGateway {
 public:
  GRPCGateway(int port);
  ~GRPCGateway();

  void ListenAndServer();

 private:
  void HttpHandle(const httplib::Request& req, httplib::Response& resp);

 private:
  google::protobuf::Service* grpc_server_ = nullptr;
  httplib::Server http_server_;
  int port_;
};

}  // namespace grpcgateway
}  // namespace kingfisher

#endif
