#include "coroutine.context.h"
#include <csignal>
#include <sys/mman.h>

namespace kingfisher {
namespace coroutine {
RoutineContext::RoutineContext(size_t stack_size, RoutineFunc f, void *args)
    //: func_(f), args_(args), stack_size_(stack_size) {
    : RoutineContextBase::RoutineContextBase(stack_size, f, args) {
  Make(f, args);
}

RoutineContext::~RoutineContext() {
  // The munmap() system call deletes the mappings for the specified address
  // range, and causes further references to addresses within the range to
  // generate invalid memory references. The region is also automatically
  // unmapped when the process is terminated. On the other hand, closing the
  // file descriptor does not unmap the region.
  // The address addr must be a multiple of the page size. All pages containing
  // a part of the indicated range are unmapped, and subsequent references to
  // these pages will generate SIGSEGV. It is not an error if the indicated
  // range does not contain any mapped pages
  munmap(stack_, stack_size_);
}

void RoutineContext::Make(RoutineFunc f, void *args) {
  getcontext(&context_);
  stack_ = mmap(NULL, stack_size_, PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  context_.uc_stack.ss_sp = stack_;
  context_.uc_stack.ss_size = stack_size_;
  context_.uc_stack.ss_flags = 0;
  context_.uc_link = getMainContext();

  uintptr_t ptr = reinterpret_cast<uintptr_t>(this);

  /* Manipulate user context UCP to continue with calling functions FUNC
     and the ARGC-1 parameters following ARGC when the context is used
     the next time in `setcontext' or `swapcontext'.

     We cannot say anything about the parameters FUNC takes; `void'
     is as good as any other choice.  */
  // makecontext(&context_, reinterpret_cast<void (*)(void)>(handle), 2,
  //            (uint32_t)ptr, (uint32_t)(ptr >> 32));
  makecontext(&context_, reinterpret_cast<void (*)(void)>(handle), 1,
              (uint32_t)ptr);
}

ucontext_t *RoutineContext::getMainContext() {
  static __thread ucontext_t main_context;
  return &main_context;
}

int RoutineContext::Resume() {
  return swapcontext(getMainContext(), &context_);
}

// Save current context in context variable pointed to by OUCP and set
// context from variable pointed to by UCP.
int RoutineContext::Yield() { return swapcontext(&context_, getMainContext()); }

// void RoutineContext::handle(uint32_t low32, uint32_t high32) {
void RoutineContext::handle(uintptr_t ptr) {
  //  uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)high32 << 32);
  //  uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)high32 << 32);
  RoutineContext *rc = reinterpret_cast<RoutineContext *>(ptr);
  if (rc) {
    if (rc->preFunc_) {
      rc->preFunc_();
    }
    if (rc->func_) {
      rc->func_(rc->args_);
    }
    if (rc->postFunc_) {
      rc->postFunc_();
    }
  }
}
}
}
