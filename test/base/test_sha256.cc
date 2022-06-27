#include <gtest/gtest.h>

#ifdef ENABLE_CRYPTO_OPENSSL
#include <iostream>
#include <string>

#include "core/scope_guard.h"
#include "crypto/sha256.h"

using namespace kingfisher;
using namespace kingfisher::crypto;

//./kingfisher_base_test  --gtest_filter=test_Sha256.*
class test_Sha256 : public testing::Test {
 public:
  test_Sha256() {}
  ~test_Sha256() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Sha256, Sha256) {
  std::string str = "abc";
  auto sh256 = kingfisher::crypto::Sha256SumString(str);
  std::cout << "sha256: " << sh256 << std::endl;
}

#endif
