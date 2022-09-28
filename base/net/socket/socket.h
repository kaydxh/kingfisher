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

  int SetNoDelay(bool flag);

  void SetReuseAddr(bool flag);

  void SetReusePort(bool flag);

  void SetKeepAlive(bool flag);

 private:
  const int sockfd_;
};
}
}

#endif
