#include "connection.h"

namespace kingfisher {
namespace net {

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
                             int sockfd, const sockets::SockAddress& local_addr,
                             const sockets::SockAddress& peer_addr)
    : loop_(loop), local_addr_(local_addr), peer_addr_(peer_addr) {}

void TcpConnection::HandleRead(time::Timestamp recv_time) {}

}  // namespace net
}  // namespace kingfisher
