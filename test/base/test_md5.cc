#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "core/scope_guard.h"
#include "crypto/md5.h"

using namespace kingfisher;
using namespace kingfisher::crypto;

//./kingfisher_base_test  --gtest_filter=test_Md5.*
class test_Md5 : public testing::Test {
 public:
  test_Md5() {}
  ~test_Md5() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Md5, Md5) {
  std::string str = "abc";
  auto sh256 = kingfisher::crypto::Md5SumString(str);
  std::cout << "md5:" << sh256 << std::endl;
}

