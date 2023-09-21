#ifndef KINGFISHER_BASE_NET_TCP_CONNECTOR_H_
#define KINGFISHER_BASE_NET_TCP_CONNECTOR_H_

#include <memory>

#include "core/noncopyable.hpp"
#include "net/event/channel.h"
#include "net/socket/socket_addr.h"

namespace kingfisher {
namespace net {

class Connector : public noncopyable_::noncopyable,
                  std::enable_shared_from_this<Connector> {
 public:
  using NewConnectionCallback = std::function<void(int sockfd)>;

  Connector(EventLoop* loop, const sockets::SockAddress& server_addr);
  ~Connector();

  void SetNewConnectionCallback(const NewConnectionCallback& cb) {
    new_connection_cb_ = cb;
  }

  void Start();

 private:
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void startInLoop();

 private:
  EventLoop* loop_;
  sockets::SockAddress server_addr_;
  std::unique_ptr<Channel> channel_;
  NewConnectionCallback new_connection_cb_;
};

}  // namespace net
}  // namespace kingfisher

#endif
