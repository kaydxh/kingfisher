#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "core/scope_guard.h"
#include "encoding/base64/base64.h"

class test_Base64 : public testing::Test {
 public:
  test_Base64() {}
  ~test_Base64() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Base64, Base64) {
  std::string str = "abc";
  auto encode_base64 = kingfisher::encoding::Base64Encode(str);
  std::cout << "encoded base64:" << encode_base64 << std::endl;

  std::string decode_base64;
  int ret = kingfisher::encoding::Base64Decode(encode_base64, decode_base64);
  std::cout << "decoded base64:" << decode_base64 << std::endl;
  EXPECT_EQ(0, ret);
  EXPECT_EQ(str, decode_base64);
}

