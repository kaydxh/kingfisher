#include "socket_stream_base.h"

// sockaddr_in
#include <netinet/in.h>
// inet_ntop
#include <arpa/inet.h>

// memset
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

//#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/tcp.h>

namespace kingfisher {
namespace net {

SocketStreamBase::SocketStreamBase(size_t buf_size)
    : std::iostream(NULL), buf_size_(buf_size) {}

SocketStreamBase::~SocketStreamBase() {}

void SocketStreamBase::NewRdbuf(SocketStreamBase* buf) {
  // rdbuf(buf);
}

bool SocketStreamBase::GetRemoteHost(char* ip, size_t size, int* port) {
  struct sockaddr_in addr;
  socklen_t slen = sizeof(addr);
  memset(&addr, 0, sizeof(addr));

  int ret = getpeername(SocketFd(), (struct sockaddr*)&addr, &slen);
  if (ret == 0) {
    inet_ntop(AF_INET, &addr, ip, size);
    if (NULL != port) {
      *port = ntohs(addr.sin_port);
    }
  }

  return ret == 0;
}

/*
bool TcpSocket::SetNonBlock(bool flag) {
  int ret = 0;
  int opt_val = fcntl(fd, F_GETFL, 0);
  if (flag) {
    ret = fcntl(socket_t, F_SETFL, opt_val | O_NONBLOCK);
  } else {
    ret = fcntl(fd, F_SETFL, opt_val & (~O_NONBLOCK));
  }

  if (0 != ret) {
  }

  return 0 == ret;
}

bool TcpSocket::SetNoDelay(int fd, bool flag) {
  int opt_val = flag ? 1 : 0;

  int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&opt_val,
                       sizeof(opt_val));
  if (0 != ret) {
  }

  return 0 == ret;
}
*/
}
}
