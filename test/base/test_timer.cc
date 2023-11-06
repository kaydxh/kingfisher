#include <gtest/gtest.h>

#include <atomic>
#include <functional>
#include <iostream>

#include "time/timer.h"
#include "time/timestamp.h"

using namespace kingfisher;
using namespace kingfisher::time;

class test_Timer : public testing::Test {
 public:
  test_Timer() {}
  ~test_Timer() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Timer, Once) {
  using Callback = std::function<void()>;
  TimerWheel timers;
  std::atomic<int> count(0);
  TimerEvent<Callback> event([&count]() {
    ++count;
    std::cout << "callback event at time: " << GetJiffies()
              << ", count: " << count << std::endl;
  });

  timers.Add(&event, 100, -1);
  EXPECT_EQ(count, 0);
  timers.Start();
  sleep(1);
  // EXPECT_EQ(count, 1);
  timers.Stop();
}

TEST_F(test_Timer, Multi) {
  using Callback = std::function<void()>;
  TimerWheel timers;
  std::atomic<int> count(0);
  TimerEvent<Callback> event([&count]() {
    ++count;
    //  std::cout << "callback event at time: " << GetJiffies()
    //           << ", count: " << count << std::endl;
  });

  timers.Add(&event, 100, 10);
  EXPECT_EQ(count, 0);
  timers.Start();
  sleep(2);
  EXPECT_EQ(count, 10);
  timers.Stop();
}

TEST_F(test_Timer, Mix) {
  using Callback = std::function<void()>;
  TimerWheel timers;
  std::atomic<int> count(0);
  // int count = 0;
  TimerEvent<Callback> event([&count]() {
    ++count;
    std::cout << "callback event1 at time: "
              << Timestamp::Now().ToFormattedString() << ", count: " << count
              << std::endl;
  });

  TimerEvent<Callback> event2([&count]() {
    ++count;
    std::cout << "callback event2 at time: "
              << Timestamp::Now().ToFormattedString() << ", count: " << count
              << std::endl;
  });

  timers.Add(&event, 6000, 10);
  timers.Add(&event2, 3000, 2);
  timers.Start();
  sleep(500);
  EXPECT_EQ(count, 30);
  timers.Stop();
}
