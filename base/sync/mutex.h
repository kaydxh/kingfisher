#ifndef BASE_SYNC_MUTEX_H_
#define BASE_SYNC_MUTEX_H_

#include <pthread.h>

#include "core/noncopyable.hpp"

namespace kingfisher {
namespace sync {

class Mutex : public noncopyable_::noncopyable {
 public:
  Mutex();
  ~Mutex();

  int Lock();
  int Unlock();

  int TryLock();

  pthread_mutex_t* PthreadMutex() { return &mutex_; }

 private:
  pthread_mutex_t mutex_;
};

class MutexGuard : public noncopyable_::noncopyable {
 public:
  MutexGuard(Mutex& mutex);
  ~MutexGuard();

 private:
  Mutex& mutex_;
};

}  // namespace sync
}  // namespace kingfisher

#endif
