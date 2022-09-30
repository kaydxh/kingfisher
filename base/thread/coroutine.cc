#include "coroutine.h"
#include <memory>
#include "coroutine.context.h"

namespace kingfisher {
namespace coroutine {

RoutinePool::RoutinePool(size_t stack_size, int n) : stack_size_(stack_size) {}

RoutinePool::~RoutinePool() {}
// return routine id
int RoutinePool::Make(RoutineFunc f, void *args) {
  if (f == nullptr) {
    return -1;
  }

  // auto index = routine_contexts_.size();
  if (last_done_routine_id_ >= 0) {
    auto index = last_done_routine_id_;
    routine_contexts_[index].routine_->Make(f, args);
    routine_contexts_[index].status_ = ROUTINE_SUSPEND;
    unfinished_routine_count++;
    return index;
  }
  std::shared_ptr<RoutineContext> new_routine(
      new RoutineContext(stack_size_, f, args));
  new_routine->SetPostRoutineFunc(
      std::bind(&RoutinePool::RoutineDoneCallback, this));
  RoutineContextSlot slot;
  slot.routine_ = new_routine;
  // slot.next_ = -1;
  slot.status_ = ROUTINE_SUSPEND;
  routine_contexts_.push_back(slot);
  unfinished_routine_count++;

  return routine_contexts_.size() - 1;
}

void RoutinePool::RoutineDoneCallback() {
  if (cur_routine_id_ != -1) {
    RoutineContextSlot &context_slot = routine_contexts_[cur_routine_id_];
    // context_slot.next_done_item = first_done_item_;
    context_slot.status_ = ROUTINE_DONE;
    last_done_routine_id_ = cur_routine_id_;
    unfinished_routine_count--;
    cur_routine_id_ = -1;
  }
}

int RoutinePool::GetUnfinishedCount() { return unfinished_routine_count; }

int RoutinePool::Resume(size_t index) {
  if (index >= routine_contexts_.size()) {
    return -1;
  }

  auto context_slot = routine_contexts_[index];
  if (context_slot.status_ == ROUTINE_SUSPEND) {
    cur_routine_id_ = index;
    context_slot.status_ = ROUTINE_RUNNING;
    return context_slot.routine_->Resume();
  }

  return 0;
}

int RoutinePool::Yield() {
  if (cur_routine_id_ != -1) {
    auto context_slot = routine_contexts_[cur_routine_id_];
    cur_routine_id_ = -1;
    context_slot.status_ = ROUTINE_SUSPEND;
    return context_slot.routine_->Yield();
  }

  return 0;
}

//
}
}
