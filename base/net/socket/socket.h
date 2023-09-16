#ifndef KINGFISHER_BASE_NET_SOCKET_SOCKET_H_
#define KINGFISHER_BASE_NET_SOCKET_SOCKET_H_

namespace kingfisher {
namespace net {

class Socket {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();
  int socket();

  int SetNonBlock(bool flag);

  int SetTcpNoDelay(bool flag);

  int SetReuseAddr(bool flag);

  int SetReusePort(bool flag);

  int SetKeepAlive(bool flag);

 private:
  const int sockfd_;
};
}  // namespace net
}  // namespace kingfisher

#endif
