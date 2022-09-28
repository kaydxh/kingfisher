#ifndef KINGFISHER_BASE_NET_SOCKET_SOCKET_STREAM_BLOCK_H_
#define KINGFISHER_BASE_NET_SOCKET_SOCKET_STREAM_BLOCK_H_

#include "stream/stream_buf_base.h"
#include "socket_stream_base.h"

namespace kingfisher {
namespace net {

class TcpStreamBufBlock : public stream::StreamBufBase {
 public:
  TcpStreamBufBlock(int socket, size_t buf_size);
  virtual ~TcpStreamBufBlock();

  ssize_t precv(void *buf, size_t len, int flags);
  ssize_t psend(const void *buf, size_t len, int flags);

 private:
  int socket_;
};

class TcpStreamBlock : public SocketStreamBase {
 public:
  TcpStreamBlock(size_t buf_size = 1024);
  ~TcpStreamBlock();

  void Attach(int socket);

  bool SetTimeout(int socket_timeout_ms);

  int SocketFd();

  int LastError();

 private:
  int socket_;
};
}
}

#endif
