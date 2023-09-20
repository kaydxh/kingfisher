#ifndef KINGFISHER_BASE_NET_EVENT_EVENT_LOOP_THREADH_POOL_
#define KINGFISHER_BASE_NET_EVENT_EVENT_LOOP_THREADH_POOL_

#include "core/noncopyable.hpp"
#include "net/event/event_loop.h"
#include "net/event/event_loop_thread.h"
namespace kingfisher {
namespace net {

class EventLoopThreadPool : public noncopyable_::noncopyable {
 public:
  EventLoopThreadPool(EventLoop* loop, int pool_size = 0);
  ~EventLoopThreadPool();

  void Start();

  EventLoop* GetNextLoop();

 private:
  EventLoop* loop_;
  int pool_size_ = 0;
  size_t next_index_ = 0;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};

}  // namespace net
}  // namespace kingfisher

#endif
