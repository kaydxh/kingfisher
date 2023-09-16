#ifndef KINGFISHER_BASE_NET_SOCKET_SOCKET_ADDR_H_
#define KINGFISHER_BASE_NET_SOCKET_SOCKET_ADDR_H_

#include <netinet/in.h>

#include "socket.ops.h"

namespace kingfisher {
namespace net {
namespace sockets {

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

class SockAddress {
 public:
  explicit SockAddress(const struct sockaddr_in& addr) : addr_(addr) {}
  explicit SockAddress(const struct sockaddr_in6& addr) : addr6_(addr) {}

  const struct sockaddr* SockAddr() const { return sockaddr_cast(&addr6_); }
  std::string ToIP() const;

 private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}  // namespace sockets
}  // namespace net
}  // namespace kingfisher

#endif
