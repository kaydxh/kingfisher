#include "tcp_server.h"

#include <iostream>
#include <memory>
#include <string>

#include "log/config.h"
#include "net/event/callback.h"
#include "net/event/event_loop.h"
#include "net/socket/socket.ops.h"
#include "net/socket/socket_addr.h"
#include "net/tcp/acceptor.h"

namespace kingfisher {
namespace net {

void defaultConnectionCallback(const TcpConnectionPtr& conn) {
  LOG(INFO) << "===========connection call back";
}

void defaultMessageCallback(const TcpConnectionPtr&, NetBuffer* buf) {
  LOG(INFO) << "===========message call back";
}

TcpServer::TcpServer(EventLoop* loop, const sockets::SockAddress& listen_addr,
                     int thread_num)
    : loop_(loop),
      acceptor_(std::make_unique<Acceptor>(loop, listen_addr)),
      thread_pool_(std::make_shared<EventLoopThreadPool>(loop, thread_num)),
      connection_cb_(defaultConnectionCallback),
      message_cb_(defaultMessageCallback) {
  acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
                                                std::placeholders::_1,
                                                std::placeholders::_2));
}

TcpServer::~TcpServer() {
  loop_->AssertInLoopThread();
  for (auto iter = conns_.begin(); iter != conns_.end(); ++iter) {
    iter->second->GetEventLoop()->RunInLoop(
        std::bind(&TcpConnection::DestoryConnection, iter->second));
    iter->second.reset();
  }
}

void TcpServer::Start() {
  thread_pool_->Start();
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

  LOG(INFO) << ">>> Tcp server newConnection: " << conn->Name() << std::endl;
  conns_[name] = conn;
  conn->SetConnectionCallback(connection_cb_);
  conn->SetMessageCallback(message_cb_);
  conn->SetCloseCallback(std::bind(&TcpServer::removeConnection, this, conn));
  io_loop->RunInLoop(std::bind(&TcpConnection::ConnectEstablished, conn));

  next_conn_id_++;
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
  loop_->RunInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
  loop_->AssertInLoopThread();
  LOG(INFO) << "Tcp server remove connnection: " << conn->Name();
  int n = conns_.erase(conn->Name());
  if (n == 0) {
    LOG(ERROR) << "not find conn: " << conn->Name() << " in TcpServer";
  }
  EventLoop* io_loop = conn->GetEventLoop();
  io_loop->QueueInLoop(std::bind(&TcpConnection::DestoryConnection, conn));
}

}  // namespace net
}  // namespace kingfisher
