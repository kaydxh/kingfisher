#include "socket_stream_block.h"

// recv and send
#include <sys/socket.h>

// close
#include <unistd.h>

// strerror
#include <string.h>

namespace kingfisher {
namespace net {

TcpStreamBufBlock::TcpStreamBufBlock(int socket, size_t buf_size)
    : StreamBufBase(buf_size), socket_(socket) {}

TcpStreamBufBlock::~TcpStreamBufBlock() {}

ssize_t TcpStreamBufBlock::precv(void *buf, size_t len, int flags) {
  return recv(socket_, buf, len, flags);
}

ssize_t TcpStreamBufBlock::psend(const void *buf, size_t len, int flags) {
  return send(socket_, buf, len, flags);
}

// TcpStreamBlock
TcpStreamBlock::TcpStreamBlock(size_t buf_size)
    : SocketStreamBase(buf_size), socket_(-1) {}

TcpStreamBlock::~TcpStreamBlock() {
  if (socket_ >= 0) {
    close(socket_);
  }
}

void TcpStreamBlock::Attach(int socket) {
  //  NewRdbuf(new TcpStreamBufBlock(socket, buf_size_));
  socket_ = socket;
}

bool TcpStreamBlock::SetTimeout(int socket_timeout_ms) {
  if (socket_ < 0) {
    return false;
  }

  struct timeval to;
  to.tv_sec = socket_timeout_ms / 1000;
  to.tv_usec = (socket_timeout_ms % 1000) * 1000;
  if (setsockopt(SocketFd(), SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to)) < 0) {
    printf("setsockopt(%d) errno %d, %s", SocketFd(), errno, strerror(errno));
    return false;
  }
  if (setsockopt(SocketFd(), SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to)) < 0) {
    printf("setsockopt(%d) errno %d, %s", SocketFd(), errno, strerror(errno));
    return false;
  }

  return true;
}

int TcpStreamBlock::SocketFd() { return socket_; }

int TcpStreamBlock::LastError() { return errno; }
}
}
