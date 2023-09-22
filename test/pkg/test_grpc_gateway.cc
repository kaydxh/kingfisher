#include <gtest/gtest.h>

#include "grpc-gateway/grpc_gateway.h"

using namespace kingfisher::grpcgateway;

class test_GRPCGateway : public testing::Test {
 public:
  test_GRPCGateway() {}
  ~test_GRPCGateway() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_GRPCGateway, All) {
  GRPCGateway grpcgateway(10000);
  grpcgateway.ListenAndServer();
}
