#include "geometry.h"

#include "cv/types/types.h"

namespace kingfisher {
namespace kcv {

void Geometry::RotatePoint(const cv::Size& origin_size,
                           const cv::Point2f& origin, int angle,
                           cv::Point2f& output) {
  if (angle == ROTATE_CLOCKWISE_0 || angle == ROTATE_CLOCKWISE_360) {
    output = cv::Point2f(origin);
    return;
  }
  if (angle == ROTATE_CLOCKWISE_90) {
    output = cv::Point2f(origin_size.height - origin.y, origin.x);
    return;
  }
  if (angle == ROTATE_CLOCKWISE_180) {
    output = cv::Point2f(origin_size.width - origin.x,
                         origin_size.height - origin.y);
    return;
  }
  if (angle == ROTATE_CLOCKWISE_270) {
    output = cv::Point2f(origin.y, origin_size.width - origin.x);
    return;
  }
  output = cv::Point2f(origin);
}

void Geometry::RotateRect(const cv::Size& origin_size, const cv::Rect& origin,
                          int angle, cv::Rect& output) {
  std::vector<cv::Point2f> corners = {
      cv::Point2f(origin.x, origin.y),
      cv::Point2f(origin.x + origin.width, origin.y),
      cv::Point2f(origin.x + origin.width, origin.y + origin.height),
      cv::Point2f(origin.x, origin.y + origin.height)};

  std::vector<cv::Point2f> rotated_corners;
  for (const auto& corner : corners) {
    cv::Point2f rotated_corner;
    RotatePoint(origin_size, corner, angle, rotated_corner);
    rotated_corners.push_back(rotated_corner);
  }

  float min_x = rotated_corners[0].x;
  float max_x = rotated_corners[0].x;
  float min_y = rotated_corners[0].y;
  float max_y = rotated_corners[0].y;
  for (const auto& pt : rotated_corners) {
    if (pt.x < min_x) {
      min_x = pt.x;
    }
    if (pt.x > max_x) {
      max_x = pt.x;
    }
    if (pt.y < min_y) {
      min_y = pt.y;
    }
    if (pt.y > max_y) {
      max_y = pt.y;
    }
  }
  output = cv::Rect(cv::Point2f(min_x, min_y), cv::Point2f(max_x, max_y));
}

}  // namespace kcv
}  // namespace kingfisher
