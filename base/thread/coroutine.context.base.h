#ifndef BASE_THREAD_COROUTINE_BASE_H_
#define BASE_THREAD_COROUTINE_BASE_H_

#include <functional>

namespace kingfisher {
namespace coroutine {

// using PreRoutineFunc = std::function<void()>;
using RoutineFunc = std::function<void(void *)>;
// using PostRoutineFunc = std::function<void()>;

class RoutineContextBase {
 public:
  RoutineContextBase() {}
  virtual ~RoutineContextBase() {}

  virtual void Make(RoutineFunc handle, void *args) = 0;
  virtual int Resume() = 0;
  virtual int Yield() = 0;
};
}
}

#endif
