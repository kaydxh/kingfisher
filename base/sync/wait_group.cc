#include "wait_group.h"

namespace kingfisher {
namespace sync {

WaitGroup::WaitGroup() {}
WaitGroup::~WaitGroup() {}

void WaitGroup::Add(int delta) { counter_ += delta; }

void WaitGroup::Done() {
  counter_--;
  if (counter_ <= 0) {
    cond_.notify_all();
  }
}

void WaitGroup::Wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [&]() { return counter_ <= 0; });
}

}  // namespace sync
}  // namespace kingfisher
