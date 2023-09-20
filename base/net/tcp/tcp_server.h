#ifndef KINGFISHER_BASE_NET_TCP_TCP_SERVER_H_
#define KINGFISHER_BASE_NET_TCP_TCP_SERVER_H_

#include <algorithm>
#include <map>
#include <memory>

#include "acceptor.h"
#include "core/noncopyable.hpp"
#include "net/event/callback.h"
#include "net/event/event_loop.h"
#include "net/event/event_loop_thread_pool.h"
#include "net/socket/socket_addr.h"

namespace kingfisher {
namespace net {

class TcpServer : public noncopyable_::noncopyable {
 public:
  TcpServer(EventLoop *loop, const sockets::SockAddress &listen_addr);
  ~TcpServer();

  void Start();

 private:
  void newConnection(int sockfd, const sockets::SockAddress &peer_addr);
  void removeConnection(const TcpConnectionPtr &conn);

 private:
  EventLoop *loop_;
  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> thread_pool_;
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  int next_conn_id_ = 1;
  std::map<std::string, TcpConnectionPtr> conns_;
};

}  // namespace net
}  // namespace kingfisher

#endif
