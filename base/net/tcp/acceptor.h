#ifndef KINGFISHER_BASE_NET_TCP_ACCEPTOR_H_
#define KINGFISHER_BASE_NET_TCP_ACCEPTOR_H_

#include "core/noncopyable.hpp"
#include "net/event/channel.h"
#include "net/event/event_loop.h"
#include "net/socket/socket.h"
#include "net/socket/socket_addr.h"

namespace kingfisher {
namespace net {

class Acceptor : public noncopyable_::noncopyable {
 public:
  using NewConnectionCallback =
      std::function<void(int sockfd, const sockets::SockAddress&)>;

  Acceptor(EventLoop* loop, const sockets::SockAddress& listen_addr,
           bool reuse_port = true);
  ~Acceptor();

  void SetNewConnectionCallback(const NewConnectionCallback& cb) {
    new_connection_cb_ = cb;
  }

  void Listen();

 private:
  void handleRead();

 private:
  EventLoop* loop_;
  Socket socket_;
  Channel channel_;
  NewConnectionCallback new_connection_cb_;
};

}  // namespace net
}  // namespace kingfisher

#endif
