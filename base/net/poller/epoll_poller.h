#ifndef KINGFISHER_BASE_NET_EVENT_EPOLL_POLLER_H_
#define KINGFISHER_BASE_NET_EVENT_EPOLL_POLLER_H_

#include <sys/epoll.h>
#include <memory>
#include <vector>
#include "poller.h"

namespace kingfisher {
namespace net {

class Channel;

class EPoller : public IPoller {
 public:
  EPoller(int maxevents = 16);
  ~EPoller();
  int Poll(std::vector<std::shared_ptr<Channel>> &channels, int timeout_ms);
  int Add(std::shared_ptr<Channel> channel, int timeout_ms);
  int Update(std::shared_ptr<Channel> channel, int timeout_ms);
  int Delete(std::shared_ptr<Channel> channel, int timeout_ms);

  bool Validate() const;

 private:
  int operate(int operation, std::shared_ptr<Channel> channel);

  int epoll_fd_ = -1;
  std::vector<epoll_event> events_;
};
}
}

#endif
