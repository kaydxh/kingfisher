#include "buffer.h"

#include <netinet/in.h>

#include <algorithm>
#include <cstring>

namespace kingfisher {
namespace container {

size_t Buffer::ReadableBytes() const { return writer_index_ - reader_index_; }

size_t Buffer::WriteableBytes() const { return buffer_.size() - writer_index_; }

const char* Buffer::Peek() const { return Begin() + reader_index_; }

char* Buffer::BeginWrite() { return Begin() + writer_index_; }

const char* Buffer::BeginWrite() const { return Begin() + writer_index_; }

void Buffer::Append(const std::string& data) {
  ensureWriteSize(data.size());
  std::copy(data.c_str(), data.data() + data.size(), BeginWrite());
  // std::memcpy(BeginWrite(), data.c_str(), data.size());
  writer_index_ += data.size();
}

void Buffer::Append(const char* data, int len) {
  ensureWriteSize(len);
  std::copy(data, data + len, BeginWrite());
  writer_index_ += len;
}

void Buffer::ensureWriteSize(size_t len) {
  if (WriteableBytes() < len) {
    buffer_.resize(writer_index_ + len);
  }
}

int Buffer::Read(std::string& buffer) {
  size_t n = ReadableBytes();
  if (n > buffer.size()) {
    buffer.resize(n);
  }

  std::copy(Peek(), Peek() + ReadableBytes(),
            static_cast<char*>(&*buffer.begin()));
  reader_index_ += n;
  return 0;
}

}  // namespace container
}  // namespace kingfisher
