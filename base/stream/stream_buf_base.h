#ifndef KINGFISHER_BASE_STREAM_STREAM_BUF_BASE_H_
#define KINGFISHER_BASE_STREAM_STREAM_BUF_BASE_H_

#include <iostream>
#include <streambuf>
namespace kingfisher {
namespace stream {

class StreamBufBase : public std::streambuf {
 public:
  StreamBufBase(size_t buf_size);
  virtual ~StreamBufBase();
  int Underflow();
  int Overflow(int c = traits_type::eof());
  int Sync();

 protected:
  virtual ssize_t precv(void *buf, size_t len, int flags) = 0;
  virtual ssize_t psend(const void *buf, size_t len, int flags) = 0;

  const size_t buf_size_;
};
}
}

#endif
