#ifndef KINGFISHER_PKG_CV_GEOMETRY_H_
#define KINGFISHER_PKG_CV_GEOMETRY_H_

#include "opencv2/opencv.hpp"

namespace kingfisher {
namespace kcv {

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
