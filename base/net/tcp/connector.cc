#include "connector.h"

#include "log/config.h"
#include "net/event//event_loop.h"

namespace kingfisher {
namespace net {

Connector::Connector(EventLoop* loop, const sockets::SockAddress& server_addr)
    : loop_(loop), server_addr_(server_addr) {}

Connector::~Connector() {}

void Connector::Start() {
  loop_->RunInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
  loop_->AssertInLoopThread();
  connect();
}

void Connector::connect() {
  int sockfd = sockets::CreateNonblockingSocketOrDie(server_addr_.Family());
  int ret = sockets::Connect(sockfd, server_addr_.SockAddr());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG(ERROR) << "connect error in Connector::startInLoop " << savedErrno;
      sockets::Close(sockfd);
      break;

    default:
      LOG(ERROR) << "Unexpected error in Connector::startInLoop " << savedErrno;
      sockets::Close(sockfd);
      break;
  }
}

void Connector::connecting(int sockfd) {
  channel_.reset(new Channel(loop_, sockfd));
  channel_->SetWriteCallback(std::bind(&Connector::handleWrite, this));
  channel_->EnableWriting();
}

void Connector::handleWrite() { LOG(INFO) << "connector handleWrite"; }

}  // namespace net
}  // namespace kingfisher
