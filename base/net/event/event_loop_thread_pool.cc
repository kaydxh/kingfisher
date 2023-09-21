#include "event_loop_thread_pool.h"

#include <memory>

#include "net/event/event_loop_thread.h"

namespace kingfisher {
namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop, int pool_size)
    : loop_(loop), pool_size_(pool_size) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::Start() {
  loop_->AssertInLoopThread();
  for (int i = 0; i < pool_size_; ++i) {
    EventLoopThread* t = new EventLoopThread();
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->Start());
  }
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
  loop_->AssertInLoopThread();
  EventLoop* loop = loop_;

  if (!loops_.empty()) {
    loop = loops_[next_index_];
    ++next_index_;

    if (next_index_ >= loops_.size()) {
      next_index_ = 0;
    }
  }

  return loop;
}

}  // namespace net
}  // namespace kingfisher
