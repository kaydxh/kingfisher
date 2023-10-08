#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "config/yaml/yaml.h"
#include "core/scope_guard.h"
#include "webserver/config.h"
#include "webserver/webserver.h"
#include "webserver/webserver.pb.h"

class test_Webserver : public testing::Test {
 public:
  test_Webserver() {}
  ~test_Webserver() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Webserver, ALL) {
  std::string path = "./testdata/webserver.yaml";
  YAML::Node yaml_data = kingfisher::yaml::GetYaml(path);
  kingfisher::web::ConfigOptions opts;
  opts.node = &yaml_data;

  auto &ws = kingfisher::web::Config::NewConfig(opts).Complete().ApplyOrDie();
  ws.Run();
}

