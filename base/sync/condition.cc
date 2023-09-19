#include "condition.h"

#include <pthread.h>

#include "sync/mutex.h"

namespace kingfisher {
namespace sync {

Condition::Condition(Mutex &mutex) : mutex_(mutex) {
  pthread_cond_init(&pcond_, nullptr);
}

Condition::~Condition() { pthread_cond_destroy(&pcond_); }

void Condition::Wait() { pthread_cond_wait(&pcond_, mutex_.PthreadMutex()); }

void Condition::Signal() { pthread_cond_signal(&pcond_); }

void Condition::Broadcast() { pthread_cond_broadcast(&pcond_); }

}  // namespace sync
}  // namespace kingfisher
