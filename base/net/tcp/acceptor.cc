#include "acceptor.h"

#include "net/event/channel.h"
#include "net/socket/socket.ops.h"
#include "net/socket/socket_addr.h"

namespace kingfisher {
namespace net {

Acceptor::Acceptor(EventLoop* loop, const sockets::SockAddress& listen_addr,
                   bool reuse_port)
    : loop_(loop),
      socket_(sockets::CreateNonblockingSocketOrDie(listen_addr.Family())),
      channel_(loop, socket_.Fd()) {
  socket_.SetReusePort(reuse_port);
  socket_.BindOrDie(listen_addr);
  // channel_.SetReadCallback();
}

Acceptor::~Acceptor() {}

void Acceptor::Listen() {
  loop_->AssertInLoopThread();
  socket_.ListenOrDie();
}

void Acceptor::handleRead() {
  loop_->AssertInLoopThread();
  sockets::SockAddress peer_addr;
  int connfd = socket_.Accept(&peer_addr);
  if (connfd >= 0) {
    if (new_connection_cb_) {
      new_connection_cb_(connfd, peer_addr);
    }
  }
}

}  // namespace net
}  // namespace kingfisher
