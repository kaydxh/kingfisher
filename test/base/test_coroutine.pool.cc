#include <gtest/gtest.h>
#include "thread/coroutine.h"

using namespace kingfisher;
using namespace kingfisher::coroutine;

class test_RoutinePool : public testing::Test {
 public:
  test_RoutinePool() {}
  ~test_RoutinePool() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

struct TaskArg {
  int seq;
  int routine_id;
  RoutinePool *pool;
};

void task(void *args) {
  TaskArg *arg = static_cast<TaskArg *>(args);
  std::cout << "routine id: " << arg->routine_id << ", seq: " << arg->seq
            << " start..." << std::endl;
  for (int i = 0; i < 8; i++) {
    std::cout << "routine id: " << arg->routine_id << ", seq: " << arg->seq
              << ", i: " << i << std::endl;
    arg->pool->Yield();
  }
  std::cout << "routine id: " << arg->routine_id << ", seq: " << arg->seq
            << " end" << std::endl;
}

TEST(test_RoutinePool, RoutinePool) {

  RoutinePool pool(64 * 1024, 0);
  int n = 4;
  std::vector<TaskArg> args(n);
  for (int i = 0; i < n; ++i) {
    args[i].pool = &pool;
    args[i].seq = i;
    args[i].routine_id = pool.Make(task, &args[i]);
  }

  while (pool.GetUnfinishedCount() > 0) {
    int seq = random() % n;
    pool.Resume(args[seq].routine_id);
  }
}
