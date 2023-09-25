#include <gtest/gtest.h>
#include <unistd.h>

#include "net/event/event_loop.h"
#include "net/event/event_loop_thread.h"
#include "thread/thread.h"

using namespace kingfisher;
using namespace kingfisher::net;
using namespace kingfisher::thread;

class test_EventLoopThread : public testing::Test {
 public:
  test_EventLoopThread() {}
  ~test_EventLoopThread() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

static void task(EventLoop* p = nullptr) {
  printf("task do in pid:%d, tid:%d , loop:%p\n", getpid(), GetTid(), p);
}

TEST_F(test_EventLoopThread, All) {
  // EventLoop loop;
  task();
#if 0
  {
    EventLoopThread t1;
  }
#endif
#if 1
  {
    EventLoopThread t2;
    EventLoop* loop = t2.Start();
    loop->RunInLoop(std::bind(task, loop));
    sleep(2);
  }
#endif
}

