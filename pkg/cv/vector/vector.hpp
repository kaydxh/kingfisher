#ifndef KINGFISHER_PKG_CV_VECTOR_H_
#define KINGFISHER_PKG_CV_VECTOR_H_

#include <cmath>
#include <cstdint>
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
void VectorToString(const std::vector<T>& data, std::string& v) {
  v.assign((const char*)data.data(), data.size() * sizeof(T));
}

template <typename T>
T Dot(const std::vector<T>& v1, const std::vector<T>& v2) {
  double r = 0;
  for (size_t i = 0; i < std::min(v1.size(), v2.size()); i++) {
    r += v1[i] * v2[i];
  }
  return static_cast<T>(r);
}

enum VECTOR_TYPE {
  VECTOR_TYPE_FLOAT = 0,
  VECTOR_TYPE_INT8 = 1,
  VECTOR_TYPE_INT7 = 2,
};

void VectorFloatToInt(const std::vector<float>& data, VECTOR_TYPE vector_type,
                      float scale, std::vector<int8_t>& v) {
  float m = sqrt(Dot<float>(data, data));
  for (float d : data) {
    // normalize
    m = d / m;
    // cast to int7
    float mapped_value = m * scale;
    int rounded_value = static_cast<int>(round(mapped_value));
    int clamped_value = 0;
    if (vector_type == VECTOR_TYPE_INT8) {
      clamped_value = std::max(-127, std::min(rounded_value, 127));
    } else if (vector_type == VECTOR_TYPE_INT7) {
      clamped_value = std::max(-63, std::min(rounded_value, 63));
    }
    v.emplace_back(static_cast<int8_t>(clamped_value));
  }
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
