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

EventLoop *g_loop;
void threadFunc() {
  printf("threadFunc(): pid = %d, tid = %d\n", getpid(), thread::GetCacheTid());

  // assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
  EventLoop loop;
  g_loop = &loop;
  // assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
  // loop.runAfter(1.0, callback);
  loop.Run();
  // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

TEST_F(test_EventLoop, EventLoop) {
  // EventLoop loop;
  Thread t(threadFunc);
  std::cout << "tid1: " << t.thread_id() << std::endl;
  std::cout << "tid2: " << thread::GetCacheTid() << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  std::cout << "start quit " << std::endl;
  g_loop->Quit();
  t.join();
  std::cout << "end quit " << std::endl;
  // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  // loop.Run();
}

