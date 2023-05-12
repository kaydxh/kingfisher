#include "coroutine.context.base.h"

namespace kingfisher {
namespace coroutine {

RoutineContextBase::RoutineContextBase(size_t stack_size, RoutineFunc f,
                                       void *args)
    : func_(f), args_(args), stack_size_(stack_size) {}

void RoutineContextBase::SetPreRoutineFunc(PreRoutineFunc preFunc) {
  preFunc_ = preFunc;
}

void RoutineContextBase::SetPostRoutineFunc(PostRoutineFunc postFunc) {
  postFunc_ = postFunc;
}
}
}
