#include "socket.ops.h"
// close
#include <unistd.h>
#include <iostream>

#include <sstream>

namespace kingfisher {
namespace net {
namespace sockets {

//
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr) {
  return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
}

struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr) {
  return static_cast<struct sockaddr*>(static_cast<void*>(addr));
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr) {
  return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
}

const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr) {
  return static_cast<const struct sockaddr_in*>(static_cast<const void*>(addr));
}

const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr) {
  return static_cast<const struct sockaddr_in6*>(
      static_cast<const void*>(addr));
}

int CreateNonblockingSocketOrDie(sa_family_t family) {
  int sockfd =
      ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (sockfd < 0) {
    // todo fatal
    std::cout << "sockets::createNonblockingSocketOrDie" << std::endl;
  }

  return sockfd;
}

int Connect(int sockfd, const struct sockaddr* addr) {
  return ::connect(sockfd, addr,
                   static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

void BindOrDie(int sockfd, const struct sockaddr* addr) {
  int ret =
      ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  if (ret < 0) {
    std::cout << "sockets::bindOrDie";
  }
  return;
}

void ListenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    std::cout << "sockets::listenOrDie";
  }
  return;
}

int Accept(int sockfd, struct sockaddr_in6* addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
  int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0) {
    int savedErrno = errno;
    std::cout << "Socket::accept";
    switch (savedErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:
      case EPERM:
      case EMFILE:  // per-process limit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        std::cout << "unexpected error of ::accept " << savedErrno;
        break;
      default:
        std::cout << "unknown error of ::accept " << savedErrno;
        break;
    }
  }
  return connfd;
}

ssize_t read(int sockfd, void* buf, size_t count) {
  return ::read(sockfd, buf, count);
}

void Close(int sockfd) {
  if (::close(sockfd) < 0) {
    std::cout << "sockets::close";
  }
}

std::string ToIpPort(const struct sockaddr* addr) {

  std::stringstream result;
  std::string ip = ToIp(addr);
  result << ip;
  uint16_t port = 0;
  if (addr->sa_family == AF_INET6) {
    const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
    port = be16toh(addr6->sin6_port);
  } else {
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    port = be16toh(addr4->sin_port);
  }
  result << ":" << port;
  return result.str();
}

std::string ToIp(const struct sockaddr* addr) {
  if (addr->sa_family == AF_INET) {
    char buf[INET_ADDRSTRLEN] = {0};
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf,
                static_cast<socklen_t>(INET_ADDRSTRLEN));
    return std::string(buf);
  }
  if (addr->sa_family == AF_INET6) {
    char buf[INET6_ADDRSTRLEN] = {0};
    const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
    ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf,
                static_cast<socklen_t>(INET6_ADDRSTRLEN));
    return std::string(buf);
  }

  return "";
}
}
}
}
