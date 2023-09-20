#include "tcp_server.h"

#include <memory>

#include "net/event/event_loop.h"
#include "net/tcp/acceptor.h"

namespace kingfisher {
namespace net {

TcpServer::TcpServer(EventLoop* loop, const sockets::SockAddress& listen_addr)
    : loop_(loop), acceptor_(std::make_unique<Acceptor>(loop, listen_addr)) {}

void TcpServer::Start() {
  loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
}

}  // namespace net
}  // namespace kingfisher
