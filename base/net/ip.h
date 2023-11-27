#ifndef KINGFISHER_BASE_NET_IP_H_
#define KINGFISHER_BASE_NET_IP_H_

#include <iostream>

namespace kingfisher {
namespace net {

class IPAddress {
 public:
  IPAddress() {}
  ~IPAddress(){};

  static int GetHostIP(std::string& ip_out);
  static int SpliteHostPort(std::string& host, int& port,
                            const std::string& addr);
};

}  // namespace net
}  // namespace kingfisher

#endif
