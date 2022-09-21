#include <gtest/gtest.h>
#include "net/url.h"

using namespace kingfisher;
using namespace kingfisher::net;

class test_URL : public testing::Test {
 public:
  test_URL() {}
  ~test_URL() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_URL, URLEncode) {
  std::string url = "http://localhost/test.php?id=2";
  std::string url_encode = URLEncode(url);
  std::cout << "url_encode: " << url_encode << std::endl;
}
