#include "grpc_gateway.h"

#include "log/config.h"

namespace kingfisher {
namespace grpcgateway {

GRPCGateway::GRPCGateway(int port) : port_(port) {}
GRPCGateway::~GRPCGateway() {}

void GRPCGateway::HttpHandle(const httplib::Request& req,
                             httplib::Response& resp) {
  LOG(INFO) << "recv method:" << req.method << ", path:" << req.path;
}

void GRPCGateway::ListenAndServer() {
  http_server_.Get(".*", [&](const httplib::Request& req,
                             httplib::Response& res) { HttpHandle(req, res); });

  http_server_.listen("0.0.0.0", port_);
}

}  // namespace grpcgateway
}  // namespace kingfisher
