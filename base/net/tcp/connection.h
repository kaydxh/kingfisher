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

  ~TcpConnection();

  void Send(const void* data, size_t len);
  void Send(const std::string& data);

  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_cb_ = cb;
  }
  void SetMessageCallback(const MessageCallback& cb) { message_cb_ = cb; }
  void SetCloseCallback(const CloseCallback& cb) { close_cb_ = cb; }

  const std::string& Name() const { return name_; }
  EventLoop* GetEventLoop() const { return loop_; }

  void DestoryConnection();

  void ConnectEstablished();
  void Shutdown();

 private:
  void handleRead();
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const void* data, size_t len);
  void shutdownInLoop();

 private:
  EventLoop* loop_;
  std::string name_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const sockets::SockAddress local_addr_;
  const sockets::SockAddress peer_addr_;

  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  CloseCallback close_cb_;

  NetBuffer input_buffer_;
  NetBuffer output_buffer_;
};

}  // namespace net
}  // namespace kingfisher

#endif
