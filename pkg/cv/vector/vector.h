#ifndef KINGFISHER_PKG_CV_VECTOR_H_
#define KINGFISHER_PKG_CV_VECTOR_H_

#include <vector>

namespace kingfisher {
namespace cv {

template <typename T>
class Vector {
 public:
  Vector(std::vector<T>& vector) : vector_(vector) {}
  ~Vector() {}

  void Norm();

  int Dims();

 private:
  std::vector<T> vector_;
};

}  // namespace cv
}  // namespace kingfisher
   //
#endif
