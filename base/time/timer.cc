
#include "time/timer.h"

#include <cmath>

#include "log/config.h"
#include "strings/strings.h"
#include "sync/mutex.h"

namespace kingfisher {
namespace time {

// implementation
// TimerEventBase
void TimerEventBase::relink(TimerWheelSlot* new_slot) {
  if (new_slot == slot_) {
    return;
  }
  if (slot_) {
    auto prev = prev_;
    auto next = next_;
    if (next) {
      next->prev_ = prev;
    }

    if (prev) {
      prev->next_ = next;
    } else {
      // Must be at head of slot. Move the next item to the head
      slot_->events_ = next;
    }
  }

  {
    if (new_slot) {
      auto old = new_slot->events_;
      next_ = old;
      if (old) {
        old->prev_ = this;
      }
      new_slot->events_ = this;
    } else {
      next_ = nullptr;
    }

    // head of slot, set prev_ to nullptr
    prev_ = nullptr;
  }

  slot_ = new_slot;
}

void TimerEventBase::cancel() {
  if (!slot_) {
    return;
  }

  relink(nullptr);
}

// TimerWheel
void TimerWheel::Start() {
  thread_pool_.start();
  thread_pool_.AddTask(&TimerWheel::runLoop, this);
}

void TimerWheel::runLoop() {
  // uint64_t remain = precision_;
  while (running_.load()) {
    MsSleep(precision_);
    schedule();
  }
}

void TimerWheel::Join() { thread_pool_.join(); }

bool TimerWheel::schedule() {
  LOCK_GUARD(lock_);
  auto slot = &slots_[cur_slot_index_];
  cur_slot_index_ = (cur_slot_index_ + 1) % num_slots_;

  while (slot->events()) {
    auto event = slot->popEvent();
    thread_pool_.AddTask(&TimerEventBase::run, event);
    now_ = GetJiffies();
    if (event->repeated_times_ == 1) {
      ;  // do nothing

    } else if (event->repeated_times_ > 1) {
      addNolock(event, event->interval_, --event->repeated_times_);

    } else {  // (event->repeated_times_ <= 0)
      addNolock(event, event->interval_, -1);
    }
  }

  return true;
}

void TimerWheel::Stop() {
  // running_ = false;
  running_.store(false);
  thread_pool_.stop();
}

int TimerWheel::Add(TimerEventBase* event, Tick interval,
                    int32_t repeated_times) {
  size_t idx = std::ceil(static_cast<float>(interval) / precision_);
  if (idx == 0) {
    // 向上取整,往后一格
    idx = 1;
  }
  if (idx >= num_slots_) {
    LOG(ERROR) << strings::FormatString("interval: %d should be <= %d",
                                        interval,
                                        precision_ * (num_slots_ - 1));
    return -1;
  }

  {
    LOCK_GUARD(lock_);
    size_t slot_index = (cur_slot_index_ + idx) % num_slots_;

    event->interval_ = interval;
    event->repeated_times_ = repeated_times;
    LOG(INFO) << strings::FormatString("add event: %p on slot_index %d", event,
                                       slot_index);
    auto slot = &slots_[slot_index];
    event->relink(slot);
  }

  return 0;
}

int TimerWheel::addNolock(TimerEventBase* event, Tick interval,
                          int32_t repeated_times) {
  size_t idx = std::ceil(static_cast<float>(interval) / precision_);
  if (idx == 0) {
    // 向上取整,往后一格
    idx = 1;
  }
  if (idx >= num_slots_) {
    LOG(ERROR) << strings::FormatString("interval: %d should be <= %d",
                                        interval,
                                        precision_ * (num_slots_ - 1));
    return -1;
  }

  size_t slot_index = (cur_slot_index_ + idx) % num_slots_;

  event->interval_ = interval;
  event->repeated_times_ = repeated_times;
  std::cout << "slot_index: " << slot_index << std::endl;
  auto slot = &slots_[slot_index];
  event->relink(slot);

  return 0;
}

uint64_t GetJiffies() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (ts.tv_sec * 1000 + ts.tv_nsec / 1e6);
}

void MsSleep(uint32_t time_ms) {
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = time_ms * 1000;
  select(0, nullptr, nullptr, nullptr, &tv);

// also can use nanosleep
#if 0
    timespec t;
    t.tv_sec = time_ms / 1000;
    t.tv_nsec = (time_ms % 1000) * 1000000;
    int ret = 0;
    do {
        ret = ::nanosleep(&t, &t);
    } while (ret == -1 && errno == EINTR);
#endif
}

}  // namespace time
}  // namespace kingfisher
