#include "socket.h"

#include "socket.ops.h"

namespace kingfisher {
namespace net {

Socket::~Socket() { sockets::Close(sockfd_); }
}
}
