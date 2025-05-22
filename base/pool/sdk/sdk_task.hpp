#ifndef BASE_POOL_POOL_SDK_TASK_H_
#define BASE_POOL_POOL_SDK_TASK_H_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <string>

#include "time/timestamp.h"

#ifdef ENABLE_LIBCO
#include "sync/co_condition_variable.h"
#endif

namespace kingfisher {
namespace pool {

const int SDK_TASK_TIMEOUT = 10000;

class SDKTask {
 public:
  SDKTask(const std::string name, const std::string& session_id)
      : name_(name), session_id_(session_id) {}
  virtual ~SDKTask() {};

  void Done() {
    ts_finish_proc_ = kingfisher::time::NowUs();
    ;
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
        ret_ = SDK_TASK_TIMEOUT;
      }
    }
    ts_wakeup_ = kingfisher::time::NowUs();
    ;

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
  int64_t ts_finish_proc_;
  int64_t ts_wakeup_;

  int64_t commit_time_ = 0;

  bool done_ = false;
  std::string name_;
  std::string session_id_;

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
