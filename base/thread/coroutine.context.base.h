#ifndef BASE_THREAD_COROUTINE_BASE_H_
#define BASE_THREAD_COROUTINE_BASE_H_

#include <functional>

namespace kingfisher {
namespace coroutine {

enum {
  ROUTINE_RUNNING,
  ROUTINE_SUSPEND,
  ROUTINE_DONE
};

using PreRoutineFunc = std::function<void()>;
using RoutineFunc = std::function<void(void *)>;
using PostRoutineFunc = std::function<void()>;

class RoutineContextBase {
 public:
  RoutineContextBase(size_t stack_size, RoutineFunc f, void *args);
  virtual ~RoutineContextBase() {}

  virtual void Make(RoutineFunc handle, void *args) = 0;
  virtual int Resume() = 0;
  virtual int Yield() = 0;

  void SetPreRoutineFunc(PreRoutineFunc preFunc);
  void SetPostRoutineFunc(PostRoutineFunc postFunc);

  // private:
 protected:
  PreRoutineFunc preFunc_ = nullptr;
  RoutineFunc func_ = nullptr;
  PostRoutineFunc postFunc_ = nullptr;

  void *args_ = nullptr;
  // 64k
  size_t stack_size_ = 64 * 1024;
  void *stack_ = nullptr;
};
}
}

#endif
