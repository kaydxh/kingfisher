#include "stream_buf_base.h"

// shared_ptr
#include <memory>

namespace kingfisher {
namespace stream {

// https://izualzhy.cn/stream-buffer
StreamBufBase::StreamBufBase(size_t buf_size) : buf_size_(buf_size) {
  std::shared_ptr<char> gbuf(new char[buf_size_]);
  std::shared_ptr<char> pbuf(new char[buf_size_]);

  // Sets the values of the pointers defining the get area. Specifically, after
  // the call eback() == gbeg, gptr() == gcurr, egptr() == gend
  setg(gbuf.get(), gbuf.get(), gbuf.get());
  // Sets the values of the pointers defining the put area. Specifically,
  // after the call pbase() == pbeg, pptr() == pbeg, epptr() == pend
  setp(pbuf.get(), pbuf.get() + buf_size_);
}

StreamBufBase::~StreamBufBase() {}

int StreamBufBase::Underflow() {
  int ret = precv(eback(), buf_size_, 0);
  if (ret > 0) {
    setg(eback(), eback(), eback() + ret);
    return traits_type::to_int_type(*gptr());
  }
  return traits_type::eof();
}

int StreamBufBase::Sync() {
  int sent = 0;
  // pbase() Returns the pointer to the beginning ("base") of the put area
  // pptr() Returns the pointer to the current character (put pointer) in the
  // put area
  // epptr() Returns the pointer one past the end of the put area
  int total = pptr() - pbase();
  while (sent < total) {
    int ret = psend(pbase() + sent, total - sent, 0);
    if (ret > 0) {
      sent += ret;
      continue;
    }
    return -1;
  }

  setp(pbase(), pbase() + buf_size_);
  // void pbump( int count );
  // Repositions the put pointer (pptr()) by count characters, where count may
  // be positive or negative. No checks are done for moving the pointer outside
  // the put area [pbase(), epptr()).
  // If the pointer is advanced and then overflow() is called to flush the put
  // area to the associated character sequence, the effect is that extra count
  // characters with undefined values are output.
  pbump(0);

  return 0;
}

// writes characters to the associated output sequence from the put area
int StreamBufBase::Overflow(int c) {
  if (-1 == Sync()) {
    return traits_type::eof();
  }
  if (!traits_type::eq_int_type(c, traits_type::eof())) {
    sputc(traits_type::to_char_type(c));
  }

  return traits_type::not_eof(c);
}

// end net
}
}
