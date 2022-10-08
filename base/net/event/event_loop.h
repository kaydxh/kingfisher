#ifndef KINGFISHER_BASE_NET_EVENT_EVENT_LOOP_H_
#define KINGFISHER_BASE_NET_EVENT_EVENT_LOOP_H_

#include "net/poller/poller.h"

namespace kingfisher {
namespace net {

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  void Run();
  void Quit();

 private:
  void handleRead();

  std::unique_ptr<IPoller> poller_;
  std::unique_ptr<Channel> wakeup_channel_;
  bool quit_ = false;

  int wakeupFd_ = -1;
};
}
}

#endif
