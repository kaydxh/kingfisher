#include "net.buffer.h"

#include "socket.ops.h"

namespace kingfisher {
namespace net {

NetBuffer::NetBuffer(int n) {}

NetBuffer::~NetBuffer() {}

int NetBuffer::Read(std::string& buffer) {
  // saved an ioctl()/FIONREAD call to tell how much to read
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = WriteableBytes();
  vec[0].iov_base = Begin() + writer_index_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);
  // when there is enough space in this buffer, don't read into extrabuf.
  // when extrabuf is used, we read 128k-1 bytes at most.
  const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
  const ssize_t n = sockets::Readv(fd_, vec, iovcnt);
  if (n < 0) {
    return n;
  } else if (static_cast<size_t>(n) <= writable) {
    writer_index_ += n;
  } else {
    writer_index_ = buffer_.size();
    // Append(extrabuf, n - writable);
    Append(buffer.c_str(), n - writable);
  }
  return n;
}
}  // namespace net
}  // namespace kingfisher
