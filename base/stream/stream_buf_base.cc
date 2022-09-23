#include "stream_buf_base.h"

// shared_ptr
#include <memory>

namespace kingfisher {
namespace stream {

StreamBufBase::StreamBufBase(size_t buf_size) : buf_size_(buf_size) {
  std::shared_ptr<char> gbuf(new char[buf_size_]);
  std::shared_ptr<char> pbuf(new char[buf_size_]);

  setg(gbuf.get(), gbuf.get(), gbuf.get());
  setp(pbuf.get(), pbuf.get() + buf_size_);
}

StreamBufBase::~StreamBufBase() {}

int StreamBufBase::Underflow() {
  int ret = precv(eback(), buf_size_, 0);
  if (ret > 0) {
    setg(eback(), eback(), eback() + ret);
    return traits_type::to_int_type(*gptr());
  } else {
    return traits_type::eof();
  }
}

int StreamBufBase::Sync() {
  int sent = 0;
  int total = pptr() - pbase();
  while (sent < total) {
    int ret = psend(pbase() + sent, total - sent, 0);
    if (ret > 0) {
      sent += ret;
    } else {
      return -1;
    }
  }

  setp(pbase(), pbase() + buf_size_);
  pbump(0);

  return 0;
}

int StreamBufBase::Overflow(int c) {
  if (-1 == sync()) {
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
