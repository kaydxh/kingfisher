#ifndef BASE_THREAD_COROUTINE_CONTEXT_H_
#define BASE_THREAD_COROUTINE_CONTEXT_H_

#include "coroutine.context.base.h"
#include <functional>
#include <ucontext.h>

namespace kingfisher {
namespace coroutine {

// https://github.com/tonbit/coroutine/blob/c1a04fb144c7870a5dd8a27d4b17976e7ad4bfca/coroutine.h
class RoutineContext : public RoutineContextBase {
 public:
  RoutineContext(size_t stack_size, RoutineFunc f, void *args);
  ~RoutineContext();

  void Make(RoutineFunc f, void *args) override;
  int Yield() override;
  int Resume() override;

 private:
  // func
  // static void handle(uint32_t low32, uint32_t high32);
  static void handle(uintptr_t ptr);
  ucontext_t *getMainContext();

  // var
  ucontext_t context_;
  // RoutineFunc func_ = nullptr;
  // void *args_ = nullptr;
  // 64k
  // size_t stack_size_ = 64 * 1024;
  // void *stack_ = nullptr;
};
}
}

#endif
