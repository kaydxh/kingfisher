#include "socket_addr.h"

#include <cstring>

#include "net/socket/endian.h"

namespace kingfisher {
namespace net {
namespace sockets {

SockAddress::SockAddress(bool loopback_only) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  in_addr_t ip = INADDR_ANY;
  if (loopback_only) {
    ip = INADDR_LOOPBACK;
  }
  addr_.sin_addr.s_addr = sockets::Hton(ip);
  addr_.sin_port = sockets::Hton(10000);
}

std::string SockAddress::ToIP() const { return sockets::ToIP(SockAddr()); }

}  // namespace sockets

}  // namespace net
}  // namespace kingfisher
