#ifndef KINGFISHER_BASE_NET_TCP_TCP_CLIENT_H_
#define KINGFISHER_BASE_NET_TCP_TCP_CLIENT_H_

#include <mutex>

#include "connector.h"
#include "core/noncopyable.hpp"
#include "net/event/callback.h"
#include "net/event/channel.h"
#include "net/socket/socket_addr.h"

namespace kingfisher {
namespace net {

class TcpClient : public noncopyable_::noncopyable {
 public:
  TcpClient(EventLoop *loop, const sockets::SockAddress &listen_addr);
  ~TcpClient();

  void Connect();
  void Disconnect();

  void SetConnectionCallback(const ConnectionCallback &cb) {
    connection_cb_ = cb;
  }
  void SetMessageCallback(const MessageCallback &cb) { message_cb_ = cb; }

 private:
  void newConnection(int sockfd);
  void removeConnection(const TcpConnectionPtr &conn);

 private:
  EventLoop *loop_;
  std::shared_ptr<Connector> connector_;
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  TcpConnectionPtr conn_;

  int next_conn_id_ = 1;

  std::mutex mutex_;
};

}  // namespace net
}  // namespace kingfisher

#endif
