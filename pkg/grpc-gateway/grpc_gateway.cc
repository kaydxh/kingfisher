#include "grpc_gateway.h"

#include "log/config.h"

namespace kingfisher {
namespace grpcgateway {

GRPCGateway::GRPCGateway() {}
GRPCGateway::~GRPCGateway() {}

#ifdef ENABLE_GRPC
void GRPCGateway::SetChannel(std::shared_ptr<grpc::Channel> channel) {
  channel_ = std::move(channel);
}

std::shared_ptr<grpc::Channel> GRPCGateway::CreateInProcessChannel(
    const std::string& target) {
  // 创建到本地 gRPC server 的 insecure channel
  return grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
}
#endif

void GRPCGateway::RegisterRoute(const GRPCRoute& route) {
  routes_.push_back(route);
  LOG(INFO) << "Registered gRPC gateway route: " << route.http_method << " "
            << route.http_path;
}

void GRPCGateway::InstallRoutes(httplib::Server& http_server) {
  for (auto& route : routes_) {
    if (route.http_method == "GET") {
      http_server.Get(route.http_path.c_str(),
                      [handler = route.handler](const httplib::Request& req,
                                                httplib::Response& resp) {
                        handler(req, resp);
                      });
    } else if (route.http_method == "POST") {
      http_server.Post(route.http_path.c_str(),
                       [handler = route.handler](const httplib::Request& req,
                                                 httplib::Response& resp) {
                         handler(req, resp);
                       });
    } else if (route.http_method == "PUT") {
      http_server.Put(route.http_path.c_str(),
                      [handler = route.handler](const httplib::Request& req,
                                                httplib::Response& resp) {
                        handler(req, resp);
                      });
    } else if (route.http_method == "DELETE") {
      http_server.Delete(route.http_path.c_str(),
                         [handler = route.handler](const httplib::Request& req,
                                                   httplib::Response& resp) {
                           handler(req, resp);
                         });
    } else {
      LOG(WARNING) << "Unsupported HTTP method: " << route.http_method
                   << " for route: " << route.http_path;
    }
  }

  LOG(INFO) << "Installed " << routes_.size() << " gRPC gateway routes";
}

std::string GRPCGateway::ProtoToJson(
    const google::protobuf::Message& message) {
  std::string json_string;
  google::protobuf::util::JsonPrintOptions options;
  options.add_whitespace = false;
  options.always_print_primitive_fields = true;

  auto status =
      google::protobuf::util::MessageToJsonString(message, &json_string, options);
  if (!status.ok()) {
    LOG(ERROR) << "Failed to convert protobuf to JSON: "
               << status.ToString();
    return "{}";
  }
  return json_string;
}

int GRPCGateway::JsonToProto(const std::string& json,
                             google::protobuf::Message* message) {
  google::protobuf::util::JsonParseOptions options;
  options.ignore_unknown_fields = true;

  auto status =
      google::protobuf::util::JsonStringToMessage(json, message, options);
  if (!status.ok()) {
    LOG(ERROR) << "Failed to convert JSON to protobuf: "
               << status.ToString();
    return -1;
  }
  return 0;
}

}  // namespace grpcgateway
}  // namespace kingfisher
