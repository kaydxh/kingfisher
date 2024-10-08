#include <gtest/gtest.h>

#include "cv/cvtable/cvtable.h"

using namespace kingfisher;
using namespace kingfisher::kcv;

class test_CVTable : public testing::Test {
 public:
  test_CVTable() {}
  ~test_CVTable() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_CVTable, All) {
  CVTable cv_table;
  int ret = cv_table.Init("./testdata/cv_table.txt");
  EXPECT_EQ(ret, 0);
  cv_table.Print();
}
