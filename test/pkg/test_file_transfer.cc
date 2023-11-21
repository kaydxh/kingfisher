#include <gtest/gtest.h>

#include "config/yaml/yaml.h"
#include "file-transfer/config.h"
#include "file-transfer/ft.transfer.h"
#include "log/config.h"

using namespace kingfisher;
using namespace kingfisher::file;

class test_FileTransfer : public testing::Test {
 public:
  test_FileTransfer() {}
  ~test_FileTransfer() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_FileTransfer, Download) {
  std::string path = "./testdata/ft.yaml";
  YAML::Node yaml_data = kingfisher::yaml::GetYaml(path);
  kingfisher::file::ConfigOptions opts;
  opts.node = &yaml_data;

  auto &ft = kingfisher::file::Config::NewConfig(opts).Complete().ApplyOrDie();
  std::string data;
  std::string url =
      "http://quyujiaofu-new-1300074211.cos.ap-guangzhou.myqcloud.com/hk_test/"
      "480p.jpg";
  int ret = ft.Download(data, url);
  ASSERT_EQ(ret, 0);
  LOG(INFO) << "data len: " << data.size();
}

