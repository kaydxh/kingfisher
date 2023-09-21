#include "socket.h"

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <cstring>

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

void Socket::BindOrDie(const sockets::SockAddress& addr) {
  sockets::BindOrDie(sockfd_, addr.SockAddr());
}

void Socket::ListenOrDie() { sockets::ListenOrDie(sockfd_); }

int Socket::Accept(sockets::SockAddress* peer_addr) {
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  int connfd = sockets::Accept(sockfd_, &addr);
  if (connfd >= 0) {
    peer_addr->SetSockAddr(addr);
  }
  return connfd;
}

void Socket::ShutdownWrite() { sockets::ShutdownWrite(sockfd_); }

}  // namespace net
}  // namespace kingfisher
