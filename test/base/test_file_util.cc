#include <gtest/gtest.h>
#include <iostream>
#include "core/scope_guard.h"
#include "file/file.h"

using namespace kingfisher;
using namespace kingfisher ::fileutil;

class test_FileUtil : public testing::Test {
 public:
  test_FileUtil() {}
  ~test_FileUtil() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_FileUtil, All) {
  EXPECT_TRUE(CopyFile("/etc/hosts", "./hosts"));
  EXPECT_FALSE(CopyFile("/etc/host", "./host"));

  EXPECT_TRUE(DeleteFile("./hosts"));
}
