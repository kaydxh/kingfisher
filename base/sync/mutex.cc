#include "mutex.h"

namespace kingfisher {
namespace sync {

Mutex::Mutex() { pthread_mutex_init(&mutex_, nullptr); }

Mutex::~Mutex() { pthread_mutex_destroy(&mutex_); }

int Mutex::Lock() { return pthread_mutex_lock(&mutex_); }

int Mutex::Unlock() { return pthread_mutex_unlock(&mutex_); }

int Mutex::TryLock() { return pthread_mutex_trylock(&mutex_); }

// MutexGuard
//
MutexGuard::MutexGuard(Mutex& mutex) : mutex_(mutex) { mutex_.Lock(); }

MutexGuard::~MutexGuard() { mutex_.Unlock(); }

}  // namespace sync
}  // namespace kingfisher
