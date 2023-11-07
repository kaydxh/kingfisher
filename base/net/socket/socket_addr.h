#ifndef KINGFISHER_BASE_NET_SOCKET_SOCKET_ADDR_H_
#define KINGFISHER_BASE_NET_SOCKET_SOCKET_ADDR_H_

#include <netinet/in.h>
#include <sys/socket.h>

#include "socket.ops.h"

namespace kingfisher {
namespace net {
namespace sockets {

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

class SockAddress {
 public:
  explicit SockAddress(uint16_t port = 0, bool loopback_only = false);
  explicit SockAddress(const struct sockaddr_in& addr) : addr_(addr) {}
  explicit SockAddress(const struct sockaddr_in6& addr) : addr6_(addr) {}

  const struct sockaddr* SockAddr() const { return sockaddr_cast(&addr6_); }
  int Len() { return sizeof(addr_); }
  std::string ToIP() const;

  void SetSockAddr(const struct sockaddr_in6& addr) { addr6_ = addr; }

  sa_family_t Family() const { return addr_.sin_family; }

 private:
  //     /* Structure describing an Internet socket address.  */
  //     struct sockaddr_in {
  //         sa_family_t    sin_family; /* address family: AF_INET */
  //         uint16_t       sin_port;   /* port in network byte order */
  //         struct in_addr sin_addr;   /* internet address */
  //     };

  //     /* Internet address. */
  //     typedef uint32_t in_addr_t;
  //     struct in_addr {
  //         in_addr_t       s_addr;     /* address in network byte order */
  //     };

  //     struct sockaddr_in6 {
  //         sa_family_t     sin6_family;   /* address family: AF_INET6 */
  //         uint16_t        sin6_port;     /* port in network byte order */
  //         uint32_t        sin6_flowinfo; /* IPv6 flow information */
  //         struct in6_addr sin6_addr;     /* IPv6 address */
  //         uint32_t        sin6_scope_id; /* IPv6 scope-id */
  //     };
  //
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}  // namespace sockets
}  // namespace net
}  // namespace kingfisher

#endif
