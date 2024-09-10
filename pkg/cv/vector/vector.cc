// #include "vector.h"
#if 0

namespace kingfisher {
namespace cv {

template <typename T>
void Vector<T>::Norm() {
  T sum = 0;
  Dot(*this, sum);

  auto magnitude = sqrt(float(sum));
  for (int i = 0; i < data_.size(); ++i) {
    data_[i] = static_cast<T>(data_[i] / magnitude);
  }
}

template <typename T>
int Vector<T>::Dims() {
  return data_.size();
}

template <typename T>
int Vector<T>::Dot(const Vector<T>& other, T& result) {
  if (data_.size() != other.data_.size()) {
    return -1;
  }

  result = 0;
  for (int i = 0; i < data_.size(); i++) {
    result += data_[i] * other.data_[i];
  }
  return 0;
}

template <typename T>
float Vector<T>::CosineDistance(const Vector<T>& other) {
  if (data_.size() != other.Data().size()) {
    return -1;
  }
  T sum = 0;
  Dot(other, sum);

  float normL = 0;
  float normR = 0;

  for (int i = 0; i < data_.size(); ++i) {
    normL += float(data_[i]) * float(data_[i]);
    normR += float(other.Data()[i]) * float(other.Data()[i]);
  }

  return sum / (sqrt(normL) * sqrt(normR));
}

}  // namespace cv
}  // namespace kingfisher
#endif
