#include "grpc_client.h"

namespace kingfisher {
namespace net {

GrpcClient::GrpcClient(const std::string& addr) : addr_(addr) {}
GrpcClient::~GrpcClient() {}

std::shared_ptr<grpc::Channel> GrpcClient::MakeChannel() {
  std::shared_ptr<grpc::Channel> channel;
  grpc::ChannelArguments grpc_arguments;
  grpc_arguments.SetMaxReceiveMessageSize(-1);
  grpc_arguments.SetMaxSendMessageSize(-1);

  channel = grpc::CreateCustomChannel(addr_, grpc::InsecureChannelCredentials(),
                                      grpc_arguments);

  return channel;
}

}  // namespace net
}  // namespace kingfisher
