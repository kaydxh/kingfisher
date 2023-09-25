#include <gtest/gtest.h>
#include <unistd.h>

#include "net/event/event_loop.h"
#include "net/event/event_loop_thread_pool.h"
#include "thread/thread.h"

using namespace kingfisher;
using namespace kingfisher::net;
using namespace kingfisher::thread;

class test_EventLoopThreadPool : public testing::Test {
 public:
  test_EventLoopThreadPool() {}
  ~test_EventLoopThreadPool() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

static void task(EventLoop* p = nullptr) {
  printf("task do in pid:%d, tid:%d , loop:%p\n", getpid(), GetTid(), p);
}

TEST_F(test_EventLoopThreadPool, All) {
  // EventLoop loop;
  task();
  EventLoop loop;
#if 1
  {
    EventLoopThreadPool p1(&loop);
    p1.Start();
  }
#endif
}
