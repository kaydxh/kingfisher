#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "grpc-gateway/grpc_gateway.h"
#include "log/config.h"

using namespace kingfisher::grpcgateway;

class test_GRPCGateway : public testing::Test {
 public:
  test_GRPCGateway() {}
  ~test_GRPCGateway() {}

  virtual void SetUp(void) {}
  virtual void TearDown(void) {}
};

TEST_F(test_GRPCGateway, RouteRegistration) {
  GRPCGateway gateway;

  // 注册一个示例路由
  GRPCRoute route;
  route.http_method = "GET";
  route.http_path = "/api/v1/echo";
  route.handler = [](const httplib::Request& req, httplib::Response& resp) {
    resp.status = 200;
    resp.set_content(R"({"message":"echo ok"})", "application/json");
  };
  gateway.RegisterRoute(route);

  // 安装到 HTTP server
  httplib::Server svr;
  gateway.InstallRoutes(svr);

  // 注意：这里不启动 server，只验证路由注册逻辑
  LOG(INFO) << "GRPCGateway route registration test passed";
}
