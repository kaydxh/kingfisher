#include <gtest/gtest.h>
#include "net/event/event_loop.h"
#include "thread/thread.h"

using namespace kingfisher;
using namespace kingfisher::net;
using namespace kingfisher::thread;

class test_EventLoop : public testing::Test {
 public:
  test_EventLoop() {}
  ~test_EventLoop() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

void threadFunc() {
  printf("threadFunc(): pid = %d\n", getpid());
  // printf("threadFunc(): pid = %d, tid = %d\n", getpid(),
  // CurrentThread::tid());

  // assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
  EventLoop loop;
  // assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
  // loop.runAfter(1.0, callback);
  loop.Run();
}

TEST_F(test_EventLoop, EventLoop) {
  // EventLoop loop;
  Thread t(threadFunc);
  t.join();
  // loop.Run();
}

