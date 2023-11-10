#ifndef BASE_SYNC_WAIT_GROUP_H_
#define BASE_SYNC_WAIT_GROUP_H_

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "core/disable.h"

namespace kingfisher {
namespace sync {

class WaitGroup {
 public:
  WaitGroup();
  ~WaitGroup();

  void Add(int delta = 1);

  void Done();

  void Wait();

 private:
  DISABLE_COPY_AND_ASSIGN(WaitGroup);

 private:
  std::mutex mutex_;
  std::condition_variable cond_;
  std::atomic<int> counter_ = 0;
};
}  // namespace sync
}  // namespace kingfisher

#endif
