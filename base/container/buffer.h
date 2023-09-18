#ifndef KINNGFISHER_BASE_BUFFER_BUFFER_H_
#define KINNGFISHER_BASE_BUFFER_BUFFER_H_

#include <string>
#include <vector>

#include "core/noncopyable.hpp"

namespace kingfisher {
namespace container {

class Buffer : public noncopyable_::noncopyable {
 public:
  explicit Buffer(int n = 1024) {}

  size_t ReadableBytes() const;
  size_t WriteableBytes() const;

  const char* Peek() const;
  char* BeginWrite();
  const char* BeginWrite() const;

  void Append(const std::string& data);

 private:
  char* begin() { return &*buffer_.begin(); }
  const char* begin() const { return &*buffer_.begin(); }
  void ensureWriteSize(size_t len);

 private:
  std::vector<char> buffer_;
  size_t reader_index_ = 0;
  size_t writer_index_ = 0;
};

}  // namespace container
}  // namespace kingfisher

#endif
