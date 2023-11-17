#ifndef KINNGFISHER_BASE_BUFFER_BUFFER_H_
#define KINNGFISHER_BASE_BUFFER_BUFFER_H_

#include <string>
#include <vector>

#include "core/noncopyable.hpp"
#include "net/socket/endian.h"
#include "strings/strings.h"

namespace kingfisher {
namespace container {

class Buffer : public noncopyable_::noncopyable {
 public:
  explicit Buffer(int n = 1024) {}
  Buffer(const std::string& data) { Append(data); }

  size_t ReadableBytes() const;
  size_t WriteableBytes() const;

  const char* Peek() const;

  char* Begin() { return &*buffer_.begin(); }
  const char* Begin() const { return &*buffer_.begin(); }
  char* BeginWrite();
  const char* BeginWrite() const;

  void Append(const std::string& data);
  void Append(const char* data, int len);

  template <typename T>
  T PeekInt(bool from_network) {
    auto v = strings::ToNumber<T>(std::string(buffer_.data(), sizeof(T)));
    if (from_network) {
      return net::sockets::Ntoh<T>(v);
    }

    return v;
  }

  virtual int Read(std::string& buffer);

  size_t Size() const { return buffer_.size(); }

 private:
  void ensureWriteSize(size_t len);

 protected:
  size_t reader_index_ = 0;
  size_t writer_index_ = 0;
  int fd_;
  std::vector<char> buffer_;
};

}  // namespace container
}  // namespace kingfisher

#endif
