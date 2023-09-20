#ifndef KINGFISHER_BASE_NET_TCP_CONNECTION_H_
#define KINGFISHER_BASE_NET_TCP_CONNECTION_H_

#include <netinet/tcp.h>

#include <memory>
#include <string>

#include "core/noncopyable.hpp"
#include "net/event/callback.h"
#include "net/event/channel.h"
#include "net/event/event_loop.h"
#include "net/socket/net.buffer.h"
#include "net/socket/socket.h"
#include "net/socket/socket_addr.h"
#include "time/timestamp.h"

namespace kingfisher {
namespace net {
class TcpConnection : public noncopyable_::noncopyable,
                      std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                const sockets::SockAddress& local_addr,
                const sockets::SockAddress& peer_addr, bool keep_alive = true);

  void Send(const void* data, size_t len);

 private:
  void handleRead();
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const void* data, size_t len);

 private:
  EventLoop* loop_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const sockets::SockAddress local_addr_;
  const sockets::SockAddress peer_addr_;

  MessageCallback message_cb_;
  CloseCallback closeCallback_;

  NetBuffer input_buffer_;
  NetBuffer output_buffer_;
};

}  // namespace net
}  // namespace kingfisher

#endif
