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

TEST_F(test_Geometry, RotateRect) {
  cv::Size origin_size(720, 568);
  cv::Rect origin_rect(36, 28, 100, 150);
  cv::Rect rotated_rect;
  int angle = 90;
  Geometry::RotateRect(origin_size, origin_rect, angle, rotated_rect);

  auto message = strings::FormatString(
      "rotated rect: x=%d, y=%d, width=%d, height=%d", rotated_rect.x,
      rotated_rect.y, rotated_rect.width, rotated_rect.height);

  LOG(INFO) << message;
}
