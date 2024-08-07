#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <string>

#include "core/scope_guard.h"
#include "cv/geometry/geometry.h"
#include "log/config.h"
#include "strings/strings.h"

using namespace kingfisher;
using namespace kingfisher::kcv;

//./kingfisher_base_test --gtest_filter=test_Geometry.*
//./kingfisher_base_test --gtest_filter=test_Geometry.RotatePoint
class test_Geometry : public testing::Test {
 public:
  test_Geometry() {}
  ~test_Geometry() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Geometry, RotatePoint) {
  cv::Size origin_size(720, 568);
  cv::Point2f origin_point(360, 284);
  cv::Point2f rotated_point;
  int angle = 90;
  Geometry::RotatePoint(origin_size, origin_point, angle, rotated_point);

  auto message = strings::FormatString(
      "rotated point: x=%f, y=%f, width=%d, height=%d", rotated_point.x,
      rotated_point.y, origin_size.width, origin_size.height);

  LOG(INFO) << message;
}
