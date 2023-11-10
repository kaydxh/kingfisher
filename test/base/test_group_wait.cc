#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "log/config.h"
#include "random/random.h"
#include "strings/strings.h"
#include "sync/wait_group.h"

using namespace kingfisher;

class test_WaitGroup : public testing::Test {
 public:
  test_WaitGroup() {}
  ~test_WaitGroup() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST(test_WaitGroup, wait) {
  kingfisher::sync::WaitGroup wg;
  std::thread threads[10];
  for (int i = 0; i < 10; ++i) {
    wg.Add(1);

    threads[i] = std::thread([&wg, i]() {
      auto tm = std::chrono::milliseconds(1000 *
                                          random::Random::GetRandomRange(1, 5));
      std::this_thread::sleep_for(tm);
      LOG(INFO) << strings::FormatString("thread: %d sleep %dms", i, tm);
      wg.Done();
    });

    threads[i].detach();
  }

  wg.Wait();

  LOG(INFO) << "wait finished";
}
