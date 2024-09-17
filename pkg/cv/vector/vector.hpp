#ifndef KINGFISHER_PKG_CV_VECTOR_H_
#define KINGFISHER_PKG_CV_VECTOR_H_

#include <cmath>
#include <iostream>
#include <vector>

namespace kingfisher {
namespace cv {

template <typename T>
void StringVectorTo(const std::string& v, std::vector<T>& data) {
  const T* begin = (const T*)v.data();
  const T* end = (const T*)(v.data() + v.size());
  data.insert(data.end(), begin, end);
}

template <typename T>
class Vector {
 public:
  Vector(const std::vector<T>& data) : data_(data) {}
  ~Vector() {}

  void Norm() {
    T sum = 0;
    Dot(*this, sum);

    auto magnitude = sqrt(float(sum));
    for (std::size_t i = 0; i < data_.size(); ++i) {
      data_[i] = static_cast<T>(data_[i] / magnitude);
    }
  }

  int Dims() { return data_.size(); }

  int Dot(const Vector<T>& other, T& result) {
    if (data_.size() != other.data_.size()) {
      return -1;
    }

    result = 0;
    for (std::size_t i = 0; i < data_.size(); i++) {
      result += data_[i] * other.data_[i];
    }
    return 0;
  }

  float CosineDistance(const Vector<T>& other) {
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

  float EuclideanDistance(const Vector<T>& other) {
    if (data_.size() != other.Data().size()) {
      return -1;
    }

    float sum = 0;
    for (std::size_t i = 0; i < data_.size(); ++i) {
      sum += (data_[i] - other.Data()[i]) * (data_[i] - other.Data()[i]);
    }

    return sqrt(sum);
  }

  void Print() {
    for (auto v : data_) {
      std::cout << v << " ";
    }
    std::cout << std::endl;
  }

  const std::vector<T>& Data() { return data_; }

 private:
  std::vector<T> data_;
};

}  // namespace cv
}  // namespace kingfisher
   //

#endif
