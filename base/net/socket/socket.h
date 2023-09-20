#ifndef KINGFISHER_BASE_NET_SOCKET_SOCKET_H_
#define KINGFISHER_BASE_NET_SOCKET_SOCKET_H_

#include "socket_addr.h"

namespace kingfisher {
namespace net {

class Socket {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();
  int socket();

  int Fd() const { return sockfd_; }

  void BindOrDie(const sockets::SockAddress& addr);

  void ListenOrDie();

  int Accept(sockets::SockAddress* peer_addr);

  int SetNonBlock(bool flag);

  int SetTcpNoDelay(bool flag);

  int SetReuseAddr(bool flag);

  int SetReusePort(bool flag);

  int SetKeepAlive(bool flag);

  void ShutdownWrite();

 private:
  const int sockfd_;
};
}  // namespace net
}  // namespace kingfisher

#endif
