#ifndef KINGFISHER_PKG_CV_GEOMETRY_H_
#define KINGFISHER_PKG_CV_GEOMETRY_H_

#include "opencv2/opencv.hpp"

namespace kingfisher {
namespace kcv {

const int ROTATE_0 = 0;
const int ROTATE_CLOCKWISE_90 = 90;
const int ROTATE_CLOCKWISE_180 = 180;
const int ROTATE_CLOCKWISE_270 = 270;
const int ROTATE_CLOCKWISE_360 = 360;

class Geometry {
 public:
  static void RotatePoint(const cv::Size& origin_size,
                          const cv::Point2f& origin, int angle,
                          cv::Point2f& output);

  static void RotateRect(const cv::Size& origin_size, const cv::Rect& origin,
                         int angle, cv::Rect& output);
};

}  // namespace kcv
}  // namespace kingfisher

#endif
