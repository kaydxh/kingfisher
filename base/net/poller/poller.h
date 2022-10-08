#ifndef KINGFISHER_BASE_NET_POLL_POLLER_H_
#define KINGFISHER_BASE_NET_POLL_POLLER_H_

#include <vector>
#include <memory>

namespace kingfisher {
namespace net {

class Channel;

class IPoller {
 public:
  IPoller();
  ~IPoller();
  int Poll(std::vector<std::shared_ptr<Channel>> &channels, int timeout_ms);
  int Add(std::shared_ptr<Channel> channel, int timeout_ms);
  int Update(std::shared_ptr<Channel> channel, int timeout_ms);
  int Delete(std::shared_ptr<Channel> channel, int timeout_ms);
};
}
}

#endif
