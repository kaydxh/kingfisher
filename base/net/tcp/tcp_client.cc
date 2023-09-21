#include "tcp_client.h"

#include <memory>

#include "log/config.h"
#include "net/event//event_loop.h"
#include "net/event/callback.h"
#include "net/tcp/connection.h"

namespace kingfisher {
namespace net {

static void removeConnectionCB(EventLoop *loop, const TcpConnectionPtr &conn) {
  LOG(INFO) << "removeConnection callback";
  // connector->
}

TcpClient::TcpClient(EventLoop *loop, const sockets::SockAddress &server_addr)
    : loop_(loop), connector_(std::make_shared<Connector>(loop, server_addr)) {
  connector_->SetNewConnectionCallback(
      std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

TcpClient::~TcpClient() {
  TcpConnectionPtr conn;
  {
    std::lock_guard<std::mutex> mutex_guard_(mutex_);
    conn = conn_;
  }
  CloseCallback cb = std::bind(&removeConnectionCB, loop_, conn);
  loop_->RunInLoop(std::bind(&TcpConnection::SetCloseCallback, conn, cb));
}

void TcpClient::Connect() { connector_->Start(); }

void TcpClient::Disconnect() {
  if (conn_) {
    conn_->Shutdown();
  }
}

void TcpClient::newConnection(int sockfd) {
  loop_->AssertInLoopThread();
  sockets::SockAddress local_addr(sockets::GetLocalSockAddrFromSocket(sockfd));
  sockets::SockAddress peer_addr(sockets::GetPeerSockAddrFromSocket(sockfd));
  std::string name = std::to_string(next_conn_id_);

  TcpConnectionPtr conn = std::make_shared<TcpConnection>(
      loop_, name, sockfd, local_addr, peer_addr);
  conn->SetConnectionCallback(connection_cb_);
  conn->SetMessageCallback(message_cb_);
  conn->SetCloseCallback(std::bind(&TcpClient::removeConnection, this, conn));

  {
    std::lock_guard<std::mutex> mutex_guard_(mutex_);
    conn_ = conn;
  }

  conn->ConnectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
  loop_->AssertInLoopThread();
  LOG(INFO) << "remove connnection " << conn->Name();
  {
    std::lock_guard<std::mutex> mutex_guard_(mutex_);
    conn_.reset();
  }

  loop_->QueueInLoop(std::bind(&TcpConnection::DestoryConnection, conn));
  // todo
}

}  // namespace net
}  // namespace kingfisher
