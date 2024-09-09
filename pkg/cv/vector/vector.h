#ifndef KINGFISHER_PKG_CV_VECTOR_H_
#define KINGFISHER_PKG_CV_VECTOR_H_

#include <vector>

namespace kingfisher {
namespace cv {

template <typename T>
class Vector {
 public:
  Vector(const std::vector<T>& data) : data_(data) {}
  ~Vector() {}

  void Norm();

  int Dims();
  int Dot(const Vector<T>& other, T& result);

 private:
  std::vector<T> data_;
};

}  // namespace cv
}  // namespace kingfisher
   //
#endif
