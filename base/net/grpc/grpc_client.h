#ifndef KINGFISHER_BASE_NET_GRPC_GRPC_CLIENT_H_
#define KINGFISHER_BASE_NET_GRPC_GRPC_CLIENT_H_

#include <grpc++/security/credentials.h>
#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>

namespace kingfisher {
namespace net {

class GrpcClient {
 public:
  GrpcClient(const std::string& addr);
  ~GrpcClient();

  std::shared_ptr<grpc::Channel> MakeChannel();

  template <typename T>
  std::unique_ptr<T> MakeClient() {
    auto channel = MakeChannel();
    std::unique_ptr<T> stub(T::NewStub(channel));
    return stub;
  }

 private:
  std::string addr_;
};

}  // namespace net
}  // namespace kingfisher

#endif
