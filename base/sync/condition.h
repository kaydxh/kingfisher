#ifndef BASE_SYNC_CONDITION_H_
#define BASE_SYNC_CONDITION_H_

#include <pthread.h>

#include "core/noncopyable.hpp"
#include "sync/mutex.h"

namespace kingfisher {
namespace sync {

class Condition : public noncopyable_::noncopyable {
 public:
  explicit Condition(Mutex& mutex);
  ~Condition();

  void Wait();

  //  int WaitFor(int64_t timeout);

  void Signal();
  void Broadcast();

 private:
  pthread_cond_t pcond_;
  Mutex& mutex_;
};

}  // namespace sync
}  // namespace kingfisher

#endif
