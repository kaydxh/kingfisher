#include "event_loop_thread.h"

#include <iostream>
#include <mutex>

#include "log/config.h"
#include "net/event/event_loop.h"
#include "sync/mutex.h"

namespace kingfisher {
namespace net {

EventLoopThread::EventLoopThread()
    : thread_(std::bind(&EventLoopThread::threadFunc, this)), cond_(mutex_) {}

EventLoopThread::~EventLoopThread() {
  if (loop_) {
    loop_->Quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::Start() {
  {
    sync::MutexGuard lock_guard(mutex_);
    start_ = true;
    cond_.Signal();
  }

  EventLoop* loop = nullptr;
  {
    sync::MutexGuard lock_guard(mutex_);
    while (!loop_) {
      cond_.Wait();
    }
    loop = loop_;
  }

  return loop;
}

void EventLoopThread::threadFunc() {
  {
    sync::MutexGuard lock_guard(mutex_);
    while (!start_) {
      cond_.Wait();
    }
  }

  LOG(INFO) << "[EventLoopThread]do thread func, tid:" << thread::GetTid();

  EventLoop loop;
  {
    sync::MutexGuard lock_guard(mutex_);
    loop_ = &loop;
    cond_.Signal();
  }
  loop.Run();

  sync::MutexGuard lock_guard(mutex_);
  loop_ = nullptr;

  return;
}

}  // namespace net
}  // namespace kingfisher
