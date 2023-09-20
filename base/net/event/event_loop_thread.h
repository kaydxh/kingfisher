#ifndef KINGFISHER_BASE_NET_EVENT_EVENT_LOOP_THREADH_
#define KINGFISHER_BASE_NET_EVENT_EVENT_LOOP_THREADH_

#include <future>

#include "core/noncopyable.hpp"
#include "net/event/event_loop.h"
#include "sync/condition.h"
#include "sync/mutex.h"
#include "thread/thread.h"

namespace kingfisher {
namespace net {

class EventLoopThread : public noncopyable_::noncopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();

  EventLoop* Start();

 private:
  void threadFunc();

 private:
  EventLoop* loop_ = nullptr;
  thread::Thread thread_;
  sync::Mutex mutex_;
  sync::Condition cond_;
  bool start_ = false;
};

}  // namespace net
}  // namespace kingfisher

#endif
