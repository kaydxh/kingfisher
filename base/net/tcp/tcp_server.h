#ifndef KINGFISHER_BASE_NET_TCP_TCP_SERVER_H_
#define KINGFISHER_BASE_NET_TCP_TCP_SERVER_H_

#include <algorithm>

#include "acceptor.h"
#include "core/noncopyable.hpp"
#include "net/event/callback.h"
#include "net/event/event_loop.h"
#include "net/socket/socket_addr.h"

namespace kingfisher {
namespace net {

class TcpServer : public noncopyable_::noncopyable {
 public:
  TcpServer(EventLoop *loop, const sockets::SockAddress &listen_addr);
  ~TcpServer();

  void Start();

 private:
  EventLoop *loop_;
  std::unique_ptr<Acceptor> acceptor_;
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
};

}  // namespace net
}  // namespace kingfisher

#endif
