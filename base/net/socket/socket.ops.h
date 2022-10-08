#ifndef KINGFISHER_BASE_NET_SOCKET_SOCKET_OPS_H_
#define KINGFISHER_BASE_NET_SOCKET_SOCKET_OPS_H_

#include <string>
#include <arpa/inet.h>

namespace kingfisher {
namespace net {
namespace sockets {

int CreateNonblockingSocketOrDie(sa_family_t family);
int Connect(int sockfd, const struct sockaddr* addr);
void BindOrDie(int sockfd, const struct sockaddr* addr);
void ListenOrDie(int sockfd);
int Accept(int sockfd, struct sockaddr_in6* addr);

ssize_t read(int sockfd, void* buf, size_t count);

void Close(int sockfd);

std::string ToIpPort(const struct sockaddr* addr);
std::string ToIp(const struct sockaddr* addr);

void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);
}
}
}

#endif
