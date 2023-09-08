#include <gtest/gtest.h>

#include "example/api_date.pb.h"
#include "middleware/api/api_guard.hpp"

class test_Api_Guid : public testing::Test {
 public:
  test_Api_Guid() {}
  ~test_Api_Guid() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST(test_Api_Guid, All) {
  sea::api::seadate::NowRequest req;
  req.set_request_id("request_id");
  sea::api::seadate::NowResponse resp;
  resp.set_request_id("request_id");
  kingfisher::middleware::ApiGuard<sea::api::seadate::NowRequest,
                                   sea::api::seadate::NowResponse>
      api_guard(&req, &resp);
  sleep(1);
}
