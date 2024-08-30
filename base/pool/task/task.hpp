#ifndef BASE_POOL_POOL_TASK_TASK_H_
#define BASE_POOL_POOL_TASK_TASK_H_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <string>

#ifdef ENABLE_LIBCO
#include "sync/co_condition_variable.h"
#endif

namespace kingfisher {
namespace pool {

class Task {
 public:
  Task() {}
  virtual ~Task(){};

  void Done() {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      done_ = true;
    }

    cv_.notify_one();
  }

  void Wait(int timeout_ms = 0) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (done_) {
      return;
    }

    if (timeout_ms <= 0) {
      cv_.wait(lock, [&]() { return done_; });
    } else {
      if (!cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                        [&]() { return done_; })) {
        done_ = true;
        ret_ = -1;
      }
    }

    return;
  }

  void Reset() { done_ = false; }

 public:
  int ret_ = 0;

 private:
  // DISABLE_COPY(TASK)

 public:
#ifdef ENABLE_LIBCO
  libco::ConditionVariable cv_;
#else
  std::condition_variable cv_;
#endif
  std::mutex mutex_;

  int64_t start_proc_time_ = 0;

  int64_t commit_time_ = 0;

  bool done_ = false;
};

#if 0
template <typename Task>
class TaskPool {
 public:
 protected:
  std::vector<std::thread> pool_;
};
#endif

}  // namespace pool
}  // namespace kingfisher

#endif
