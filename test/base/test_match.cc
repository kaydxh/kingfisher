#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "path/match.h"

class test_Match : public testing::Test {
 public:
  test_Match() {}
  ~test_Match() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Match, All) {
  std::string pattern = "./logs*";
  std::vector<std::string> result = kingfisher::path::Glob(pattern);
  for (auto &e : result) {
    std::cout << e << std::endl;
  }
}
