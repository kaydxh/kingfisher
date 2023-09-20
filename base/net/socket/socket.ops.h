#ifndef KINGFISHER_BASE_NET_SOCKET_SOCKET_OPS_H_
#define KINGFISHER_BASE_NET_SOCKET_SOCKET_OPS_H_

#include <arpa/inet.h>

#include <string>

namespace kingfisher {
namespace net {
namespace sockets {

int CreateNonblockingSocketOrDie(sa_family_t family);
int Connect(int sockfd, const struct sockaddr* addr);
void BindOrDie(int sockfd, const struct sockaddr* addr);
void ListenOrDie(int sockfd);
int Accept(int sockfd, struct sockaddr_in6* addr);
int GetSocketError(int sockfd);

ssize_t Read(int sockfd, void* buf, size_t count);
ssize_t Readv(int sockfd, const struct iovec* iov, int iovcnt);
ssize_t Write(int sockfd, const void* buf, size_t count);

int Close(int sockfd);

int ShutdownWrite(int sockfd);

std::string ToIPPort(const struct sockaddr* addr);
std::string ToIP(const struct sockaddr* addr);

void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);

struct sockaddr_in6 GetLocalSockAddrFromSocket(int sockfd);
struct sockaddr_in6 GetPeerSockAddrFromSocket(int sockfd);

}  // namespace sockets
}  // namespace net
}  // namespace kingfisher

#endif
