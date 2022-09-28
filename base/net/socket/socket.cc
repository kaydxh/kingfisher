#include "socket.h"
#include <netinet/tcp.h>

#include "socket.ops.h"

namespace kingfisher {
namespace net {

Socket::~Socket() { sockets::Close(sockfd_); }

int Socket::SetTcpNoDelay(bool flag) {
  int optval = flag ? 1 : 0;
  return ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,
                      static_cast<socklen_t>(sizeof optval));
}

int Socket::SetReuseAddr(bool flag) {
  int optval = flag ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval,
                      static_cast<socklen_t>(sizeof optval));
}

int Socket::SetReusePort(bool flag) {
  int optval = flag ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval,
                      static_cast<socklen_t>(sizeof optval));
}

int Socket::SetKeepAlive(bool flag) {
  int optval = flag ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
                      static_cast<socklen_t>(sizeof optval));
}
}
}
