#include <gtest/gtest.h>

#include "config/yaml/yaml.h"
#include "log/config.h"
#include "log/log.pb.h"

class test_Log : public testing::Test {
 public:
  test_Log() {}
  ~test_Log() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST(test_Log, All) {
  std::string cfg_file = "./testdata/log.yaml";
  YAML::Node yaml_data = kingfisher::yaml::GetYaml(cfg_file);

  kingfisher::log::ConfigOptions opts;
  opts.node = &yaml_data;
  int ret = kingfisher::log::Config::NewConfig(opts)->Complete()->Install();
  EXPECT_EQ(ret, 0);
}
