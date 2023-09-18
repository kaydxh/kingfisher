#include "buffer.h"

#include <netinet/in.h>

#include <algorithm>
#include <cstring>

namespace kingfisher {
namespace container {

size_t Buffer::ReadableBytes() const { return writer_index_ - reader_index_; }

size_t Buffer::WriteableBytes() const { return buffer_.size() - writer_index_; }

const char* Buffer::Peek() const { return begin() + reader_index_; }

char* Buffer::BeginWrite() { return begin() + writer_index_; }

const char* Buffer::BeginWrite() const { return begin() + writer_index_; }

void Buffer::Append(const std::string& data) {
  ensureWriteSize(data.size());
  std::copy(data.c_str(), data.data() + data.size(), BeginWrite());
  // std::memcpy(BeginWrite(), data.c_str(), data.size());
}

void Buffer::ensureWriteSize(size_t len) {
  if (WriteableBytes() < len) {
    buffer_.resize(writer_index_ + len);
  }
}

}  // namespace container
}  // namespace kingfisher
