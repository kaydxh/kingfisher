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
  EPoller();
  ~EPoller();
  int Poll(std::vector<std::shared_ptr<Channel>> &channels, int timeout_ms);
  int Add(std::shared_ptr<Channel> channel, int timeout_ms);
  int Update(std::shared_ptr<Channel> channel, int timeout_ms);
  int Delete(std::shared_ptr<Channel> channel, int timeout_ms);

 private:
  int epoll_fd_ = -1;
  std::vector<epoll_event> events_;
};
}
}

#endif
