#include "vector.h"

#include <cmath>

namespace kingfisher {
namespace cv {

template <typename T>
void Vector<T>::Norm() {
  float sum = 0;
  for (int i = 0; i < vector_.size(); i++) {
    sum += vector_[i] * vector_[i];
  }

  auto magnitude = sqrt(float(sum));
  for (int i = 0; i < vector_.size(); ++i) {
    vector_[i] = static_cast<T>(vector_[i] / magnitude);
  }
}

template <typename T>
int Vector<T>::Dims() {
  return vector_.size();
}

}  // namespace cv
}  // namespace kingfisher
