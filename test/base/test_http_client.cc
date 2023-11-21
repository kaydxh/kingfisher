#include <gtest/gtest.h>

#include "log/config.h"
#include "net/http/http_client.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"
#include "strings/strings.h"

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
  req.SetUrl("http://127.0.0.1:26003/healthz");
  req.SetBody("test");

  HttpResponse resp;
  int ret = client.Get(req, resp);
  LOG(INFO) << strings::FormatString("get ret: %d, body: %s, body size: %d",
                                     ret, resp.Body().c_str(),
                                     resp.Body().size());

  LOG(INFO) << "headers: ";
  req.Headers().Traverse([&](const std::string& k, const std::string& v) {
    LOG(INFO) << k << ":" << v;
  });
  EXPECT_EQ(ret, 0);

  //  sleep(1);
}
