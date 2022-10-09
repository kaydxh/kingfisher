#include "thread.h"
#include <unistd.h>
#include <exception>
#include <iostream>
#include <sys/syscall.h>

namespace kingfisher {
namespace thread {

Thread::Thread(ThreadFunc func)
    : func_(func), thread_(std::bind(&Thread::run, this)) {}

Thread::~Thread() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

std::thread::id Thread::thread_id() noexcept { return thread_.get_id(); }

void Thread::run() {
  if (nullptr == func_) {
    return;
  }

  try {
    func_();
  }
  catch (std::exception &ex) {
    std::cerr << "unhandle std::exception: " << ex.what() << std::endl;
  }
  catch (...) {
    std::cerr << "unhandle unknown exception" << std::endl;
  }

  std::cout << "finish task" << std::endl;
  return;
}

void Thread::join() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

pid_t GetTid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

__thread int t_cached_tid = 0;
pid_t GetCacheTid() {
  if (t_cached_tid == 0) {
    t_cached_tid = GetTid();
  }

  return t_cached_tid;
}

bool IsMainThread() { return GetCacheTid() == getpid(); }

}  // namespace thread
}  // namespace kingfisher
