#ifndef KINGFISHER_BASE_NET_EVENT_EVENT_LOOP_H_
#define KINGFISHER_BASE_NET_EVENT_EVENT_LOOP_H_

#include <memory>
#include <functional>
#include "net/poller/poller.h"
#include <mutex>

namespace kingfisher {
namespace net {

class EventLoop {
 public:
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();

  void Wakeup();
  void Run();
  void Quit();
  bool IsInLoopThread() const;
  void RunInLoop(Functor&& cb);
  void QueueInLoop(Functor&& cb);

 private:
  void handleRead();

  std::unique_ptr<IPoller> poller_;
  bool quit_ = false;

  int wakeup_fd_ = -1;
  std::shared_ptr<Channel> wakeup_channel_;
  const pid_t thread_id_ = -1;
  std::vector<Functor> pending_functors_;
  std::mutex mutex_;
};
}
}

#endif
