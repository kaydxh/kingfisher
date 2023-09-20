#include "connection.h"

#include <memory>

#include "log/config.h"
#include "net/event/callback.h"
#include "net/event/channel.h"
#include "net/socket/socket.ops.h"

namespace kingfisher {
namespace net {

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
                             int sockfd, const sockets::SockAddress& local_addr,
                             const sockets::SockAddress& peer_addr,
                             bool keep_alive)
    : loop_(loop),
      name_(name),
      channel_(std::make_unique<Channel>(loop, sockfd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr),
      input_buffer_(sockfd) {
  channel_->SetReadCallback(std::bind(&TcpConnection::handleRead, this));
  channel_->SetWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->SetCloseCallback(std::bind(&TcpConnection::handleClose, this));

  socket_->SetKeepAlive(keep_alive);
}

void TcpConnection::Send(const void* data, size_t len) {
  sendInLoop(data, len);
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
  loop_->AssertInLoopThread();
  // todo
  ssize_t nwrote = 0;
  ssize_t remain = len;
  nwrote = sockets::Write(channel_->Fd(), data, len);
  if (nwrote >= 0) {
    remain = len - nwrote;
    LOG(INFO) << "tcp send " << nwrote << " data, remain " << remain;
  }
}

void TcpConnection::handleRead() {
  loop_->AssertInLoopThread();
  ssize_t n = input_buffer_.Read();
  if (n == 0) {
    return;
  }
  if (n > 0) {
    message_cb_(shared_from_this(), &input_buffer_);
    return;
  }
  handleError();
}

void TcpConnection::handleWrite() {
  loop_->AssertInLoopThread();
  ssize_t n = sockets::Write(channel_->Fd(), output_buffer_.Peek(),
                             output_buffer_.ReadableBytes());

  if (n > 0) {
    // todo
    return;
  }

  LOG(ERROR) << "socket write err:" << n;
}

void TcpConnection::handleClose() {
  loop_->AssertInLoopThread();
  // TcpConnectionPtr guardThis(shared_from_this());
  // closeCallback_(guardThis);
  close_cb_(shared_from_this());
}

void TcpConnection::handleError() {
  int err = sockets::GetSocketError(channel_->Fd());
  LOG(INFO) << "tcp connnection handel err:" << err;
}

void TcpConnection::DestoryConnection() {
  loop_->AssertInLoopThread();
  channel_->Remove();
}

}  // namespace net
}  // namespace kingfisher
