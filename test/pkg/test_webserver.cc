#include <gtest/gtest.h>
#include <glog/logging.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "cpp-httplib/httplib.h"
#include "config/yaml/yaml.h"
#include "webserver/config.h"
#include "webserver/controller/date/date.h"
#include "webserver/controller/healthz/healthz.h"
#include "webserver/webserver.h"

class test_Webserver : public testing::Test {
 public:
  test_Webserver() {}
  ~test_Webserver() {}

  virtual void SetUp(void) {}
  virtual void TearDown(void) {}
};

TEST_F(test_Webserver, ALL) {
  std::string path = "./test/testdata/webserver.yaml";
  YAML::Node yaml_data = kingfisher::yaml::GetYaml(path);

  // 准备 WebHandler
  std::vector<std::shared_ptr<kingfisher::web::WebHandler>> handlers;
  handlers.push_back(
      std::make_shared<kingfisher::web::HealthzWebHandler>());
  handlers.push_back(
      std::make_shared<kingfisher::web::DateWebHandler>());

  // 配置选项
  kingfisher::web::ConfigOptions opts;
  opts.node = &yaml_data;
  opts.web_handlers = handlers;

  // 创建并启动 WebServer
  auto& ws =
      kingfisher::web::Config::NewConfig(opts).Complete().ApplyOrDie();

  // 添加自定义 PostStartHook 示例
  ws.AddPostStartHook("example-hook", []() -> int {
    LOG(INFO) << "Example PostStartHook executed!";
    return 0;
  });

  // 添加自定义健康检查器
  ws.AddReadyzChecker(
      std::make_shared<kingfisher::web::FuncHealthChecker>(
          "custom-check", []() -> int { return 0; }));

  // 在后台线程运行 WebServer
  ws.Run();

#if 0
  std::thread server_thread([&ws]() {
    ws.Run();
  });


  // 等待服务器启动
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // 发送 HTTP 请求验证服务是否正常
  httplib::Client cli("127.0.0.1", 10000);
  cli.set_connection_timeout(3);

  // 测试健康检查接口
  auto res = cli.Get("/healthz");
  ASSERT_TRUE(res) << "HTTP request to /healthz failed";
  LOG(INFO) << "/healthz response status: " << res->status
            << ", body: " << res->body;
  EXPECT_EQ(res->status, 200);

  // 测试就绪检查接口
  res = cli.Get("/readyz");
  ASSERT_TRUE(res) << "HTTP request to /readyz failed";
  LOG(INFO) << "/readyz response status: " << res->status
            << ", body: " << res->body;
  EXPECT_EQ(res->status, 200);

  // 测试 date 接口
  res = cli.Get("/api/date");
  if (res) {
    LOG(INFO) << "/api/date response status: " << res->status
              << ", body: " << res->body;
  }

  // 关闭服务器
  LOG(INFO) << "Shutting down webserver...";
  ws.Shutdown();

  // 等待后台线程结束
  if (server_thread.joinable()) {
    server_thread.join();
  }
  LOG(INFO) << "Webserver shut down successfully.";
#endif
}

