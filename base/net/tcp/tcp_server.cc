#include "tcp_server.h"

#include <memory>
#include <string>

#include "connection.h"
#include "log/config.h"
#include "net/event/callback.h"
#include "net/event/event_loop.h"
#include "net/socket/socket.ops.h"
#include "net/socket/socket_addr.h"
#include "net/tcp/acceptor.h"

namespace kingfisher {
namespace net {

TcpServer::TcpServer(EventLoop* loop, const sockets::SockAddress& listen_addr,
                     int thread_num)
    : loop_(loop),
      acceptor_(std::make_unique<Acceptor>(loop, listen_addr)),
      thread_pool_(std::make_shared<EventLoopThreadPool>(loop, thread_num)) {}

TcpServer::~TcpServer() { loop_->AssertInLoopThread(); }

void TcpServer::Start() {
  loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
}

void TcpServer::newConnection(int sockfd,
                              const sockets::SockAddress& peer_addr) {
  loop_->AssertInLoopThread();
  EventLoop* io_loop = thread_pool_->GetNextLoop();
  sockets::SockAddress local_addr(sockets::GetLocalSockAddrFromSocket(sockfd));

  std::string name = std::to_string(next_conn_id_);
  TcpConnectionPtr conn = std::make_shared<TcpConnection>(
      io_loop, name, sockfd, local_addr, peer_addr);
  conns_[name] = conn;
  conn->SetConnectionCallback(connection_cb_);
  conn->SetMessageCallback(message_cb_);
  conn->SetCloseCallback(std::bind(&TcpServer::removeConnection, this, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
  loop_->AssertInLoopThread();
  LOG(INFO) << "remove connnection " << conn->Name();
  int n = conns_.erase(conn->Name());
  if (n == 0) {
    LOG(ERROR) << "not find conn: " << conn->Name() << " in TcpServer";
  }
  EventLoop* io_loop = conn->GetEventLoop();
  io_loop->QueueInLoop(std::bind(&TcpConnection::DestoryConnection, conn));
}

}  // namespace net
}  // namespace kingfisher
