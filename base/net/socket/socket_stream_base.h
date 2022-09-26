#ifndef KINGFISHER_BASE_NET_SOCKET_SOCKET_STREAM_BASE_H_
#define KINGFISHER_BASE_NET_SOCKET_SOCKET_STREAM_BASE_H_

#include <iostream>

namespace kingfisher {
namespace net {

class SocketStreamBase : public std::iostream {
 public:
  SocketStreamBase(size_t buf_size = 4096);

  virtual ~SocketStreamBase();

  virtual bool SetTimeout(int socket_timeout_ms) = 0;

  void NewRdbuf(SocketStreamBase* buf);

  bool GetRemoteHost(char* ip, size_t size, int* port = NULL);

  std::istream& getlineWithTrimRight(char* line, size_t size);

  virtual int LastError() = 0;

 protected:
  virtual int SocketFd() = 0;

  const size_t buf_size_;
};

class TcpSocket {
 public:
  static bool SetNonBlock(int fd, bool flag);

  static bool SetNoDelay(int fd, bool flag);
};
}
}

#endif
