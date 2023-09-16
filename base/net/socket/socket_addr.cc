#include "socket_addr.h"
namespace kingfisher {
namespace net {
namespace sockets {

std::string SockAddress ::ToIP() const { return sockets::ToIP(SockAddr()); }
}  // namespace sockets

}  // namespace net
}  // namespace kingfisher
