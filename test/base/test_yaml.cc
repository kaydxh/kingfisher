#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "config/yaml/yaml.h"
#include "core/scope_guard.h"
#include "example/webserver.pb.h"

class test_YAML : public testing::Test {
 public:
  test_YAML() {}
  ~test_YAML() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_YAML, ALL) {
  std::string path = "./testdata/webserver.yaml";
  // std::string path = "./testdata/example.yaml";
  YAML::Node yaml_data = kingfisher::yaml::GetYaml(path);
  // std::cout << "bind_address: "
  //          << yaml_data["web"]["bind_address"]["port"].as<std::string>()
  //         << std::endl;
  ::go::pkg::webserver::WebConfig web;
  //::go::pkg::webserver::Web web;

  // Person web;
  kingfisher::yaml::UnmarshalProtoMessage(yaml_data, web);
  std::cout << "web:" << web.DebugString() << std::endl;
}

