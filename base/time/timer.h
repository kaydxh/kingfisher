//
// Created by kayxhding on 2020-06-11 17:40:58
//

#ifndef KINNGFISHER_BASE_CORE_TIMER_H_
#define KINNGFISHER_BASE_CORE_TIMER_H_

#include <iostream>
#include <limits>
// #include <thread>
#include "core/disable.h"
#include "thread/thread_pool.h"

namespace kingfisher {
namespace time {

class TimerWheelSlot;
class TimerWheel;

using Tick = uint64_t;

uint64_t GetJiffies();
void MsSleep(uint32_t time_ms);

class TimerEventBase {
 public:
  TimerEventBase() {}
  virtual ~TimerEventBase() {}

  inline void cancel();

  bool active() const { return nullptr != slot_; }

 private:
  DISABLE_COPY_AND_ASSIGN(TimerEventBase);
  friend TimerWheelSlot;
  friend TimerWheel;
  virtual void run() = 0;
  virtual void relink(TimerWheelSlot* slot);

  void set_rotation_at(size_t rotation_at) { rotation_at_ = rotation_at; }
  void set_scheduled_at(Tick ts) { scheduled_at_ = ts; }

 private:
  TimerWheelSlot* slot_ = nullptr;
  TimerEventBase* prev_ = nullptr;
  TimerEventBase* next_ = nullptr;

  int32_t repeated_times_ = 1;
  size_t org_rotation_at_ = 0;
  size_t rotation_at_ = 0;

  Tick interval_ = 0;

  Tick scheduled_at_;
};

template <typename CBType>
class TimerEvent : public TimerEventBase {
 public:
  explicit TimerEvent<CBType>(const CBType& cb) : cb_(cb) {}

  void run() { cb_(); }

 private:
  DISABLE_COPY_AND_ASSIGN(TimerEvent);

 private:
  CBType cb_;
};

class TimerWheelSlot {
 public:
  TimerWheelSlot() {}
  ~TimerWheelSlot() {}

 private:
  // DISABLE_COPY_AND_ASSIGN(TimerWheelSlot);
  friend TimerEventBase;
  friend TimerWheel;

  // return the fist event queued in slot
  TimerEventBase* events() const { return events_; }

  // deque the first event from the slot, and return it
  TimerEventBase* popEvent() {
    auto event = events_;
    events_ = event->next_;
    if (events_) {
      events_->prev_ = nullptr;
    }
    event->next_ = nullptr;
    event->slot_ = nullptr;

    return event;
  }

 private:
  TimerEventBase* events_ = nullptr;
};

class TimerWheel {
 public:
  // default max time 3600000s = 1day
  // 最大时间间隔num_slots * precision
  TimerWheel(uint64_t num_slots = 100, Tick precision = 1)
      : num_slots_(num_slots),
        now_(GetJiffies()),
        precision_(precision),
        ticks_pending_(0) {
    slots_.resize(num_slots);
  }

  ~TimerWheel() { thread_pool_.stop(); }

  void Start();

  // a repeating timeout event that will fire every "interval" time
  void Schedule(TimerEventBase* event, Tick interval,
                int32_t repeated_times = 1);

  void Join();
  void Stop();

 private:
  void runLoop();
  bool process_current_slot();

 private:
  static const int WIDTH_BITS = 8;
  // 256
  uint64_t num_slots_;  // 1 << WIDTH_BITS;

  // 255 01111111
  Tick now_;
  Tick precision_;  // ms
  Tick ticks_pending_;
  // TimerWheelSlot slots_[num_slots_];
  std::vector<TimerWheelSlot> slots_;

  kingfisher::thread::ThreadPool thread_pool_;

  // std::thread thread_;
  bool running_ = true;
  size_t cur_slot_index_ = 0;
};

}  // namespace time
}  // namespace kingfisher
#endif
