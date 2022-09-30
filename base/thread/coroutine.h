#ifndef BASE_THREAD_COROUTINE_H_
#define BASE_THREAD_COROUTINE_H_

#include "coroutine.context.base.h"
#include <memory>
#include <vector>

namespace kingfisher {
namespace coroutine {

class RoutinePool {
 public:
  RoutinePool(size_t stack_size, int n);
  ~RoutinePool();

  int Make(RoutineFunc f, void* args);
  int Yield();
  int Resume(size_t index);

  void RoutineDoneCallback();
  int GetUnfinishedCount();

 private:
  struct RoutineContextSlot {
    std::shared_ptr<RoutineContextBase> routine_ = nullptr;
    // int next_;
    int status_ = ROUTINE_RESERVE;
  };

  size_t stack_size_ = 64 * 1024;
  std::vector<RoutineContextSlot> routine_contexts_;
  int cur_routine_id_ = -1;
  int last_done_routine_id_ = -1;
  int unfinished_routine_count = 0;
};
}
}

#endif
