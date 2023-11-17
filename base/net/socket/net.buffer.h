#ifndef KINGFISHER_BASE_NET_SOCKET_NET_BUFFER_H_
#define KINGFISHER_BASE_NET_SOCKET_NET_BUFFER_H_

#include "container/buffer.h"

namespace kingfisher {
namespace net {

class NetBuffer : public container::Buffer {
 public:
  NetBuffer(int n = 1024);
  ~NetBuffer();

  virtual int Read(std::string& buffer);

 private:
};

}  // namespace net
}  // namespace kingfisher

#endif
