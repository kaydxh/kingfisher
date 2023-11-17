#include <gtest/gtest.h>

#include "net/http/http_client.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"

using namespace kingfisher;
using namespace kingfisher::net;

class test_HttpClient : public testing::Test {
 public:
  test_HttpClient() {}
  ~test_HttpClient() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST(test_HttpClient, HttpClient) {
  HttpClient client;
  HttpRequest req;
  req.SetUrl("http://127.0.0.1:18000/Now");
  req.SetBody("test");

  HttpResponse resp;
  int ret = client.Post(req, resp);
  std::cout << "get ret: " << ret << ", body: " << resp.Body()
            << ", body size: " << resp.Body().size() << std::endl;
  EXPECT_EQ(ret, 0);

  //  sleep(1);
}
