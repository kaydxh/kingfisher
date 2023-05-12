#include <gtest/gtest.h>
#include "thread/coroutine.context.h"

using namespace kingfisher;
using namespace kingfisher::coroutine;

class test_RoutineContext : public testing::Test {
 public:
  test_RoutineContext() {}
  ~test_RoutineContext() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

void f1(void *);
void f2(void *);

RoutineContext r1(64 * 1024, &f1, nullptr);
RoutineContext r2(64 * 1024, &f2, nullptr);

int test_count = 0;
void f1(void *) {
  for (int i = 0; i < test_count; i++) {
    std::cout << "f1 resume: " << i << std::endl;
    r1.Yield();
  }
  std::cout << "f1 end" << std::endl;
}

void f2(void *) {
  for (int i = 0; i < test_count; i++) {
    std::cout << "f2 resume: " << i << std::endl;
    r2.Yield();
  }
  std::cout << "f2 end" << std::endl;
}

TEST(test_RoutineContext, RoutineContext) {

  test_count = 4;
  int test_count_2 = (test_count + 1) * 2;
  int test_count_3 = (test_count + 1);
  for (int i = 0; i < test_count_2; i++) {
    if (i < test_count_3) {
      std::cout << "start resume f1: " << i << std::endl;
      r1.Resume();
    } else {
      std::cout << "start resume f2: " << i << std::endl;
      r2.Resume();
    }
  }
}
