#include "net/ip.h"

#include <arpa/inet.h>
#include <ifaddrs.h>

#include <string>

#include "strings/strings.h"

namespace kingfisher {
namespace net {

const std::string LOCAL_LOOPBACK_IP = "127.0.0.1";

int IPAddress::GetHostIP(std::string &ip_out) {
  ifaddrs *if_addr = nullptr;
  int ret = getifaddrs(&if_addr);
  if (ret != 0) {
    return ret;
  }

  ifaddrs *if_addr_back = if_addr;
  for (; if_addr != nullptr; if_addr = if_addr->ifa_next) {
    if (if_addr->ifa_addr->sa_family != AF_INET) {
      continue;
    }

    auto addr_ptr = &((struct sockaddr_in *)if_addr->ifa_addr)->sin_addr;
    char buffer[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, addr_ptr, buffer, INET_ADDRSTRLEN);
    ip_out = buffer;
    if (0 == (ip_out.size() == 0 || ip_out.find("127.0"))) {
      continue;
    }
    break;
  }

  if (if_addr_back) {
    freeifaddrs(if_addr_back);
  }

  if (0 == ip_out.size()) {
    ip_out = LOCAL_LOOPBACK_IP;
  }

  return 0;
}

int IPAddress::SpliteHostPort(std::string &host, int &port,
                              const std::string &addr) {
  size_t pos = addr.find(":");
  if (pos != std::string::npos &&
      addr.find(":", pos + 1) == std::string::npos) {
    // only one colon
    host = addr.substr(0, pos);
    auto s_port = addr.substr(pos + 1, addr.size() - pos - 1);
    int64_t n_port = 0;
    int ret = strings::ParseInt64(n_port, s_port);
    if (ret != 0) {
      return ret;
    }
    port = static_cast<int>(n_port);
  } else {
    // 0 or more than one colons
    host = addr;
    port = 0;
  }

  return 0;
}

}  // namespace net
}  // namespace kingfisher
