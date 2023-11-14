#include <gtest/gtest.h>

#include "thread/thread_pool.h"

using namespace kingfisher;
using namespace kingfisher::thread;

class test_ThreadPool : public testing::Test {
 public:
  test_ThreadPool() {}
  ~test_ThreadPool() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

void taskAdd(int a, int b) {
  a += b;
  std::cout << "do worker task add result= " << a << std::endl;
}

int taskAdd2(int a, int b) {
  a += b;
  sleep(2);
  std::cout << "do worker task add result= " << a << std::endl;
  return a;
}

class Worker {
 public:
  void task(int a, int b) {
    a += b;
    std::cout << "do worker task add result= " << a << std::endl;
  }
};

TEST(test_ThreadPool, theadpool) {
  {
    ThreadPool pool(4);
    pool.start();
    pool.AddTask(taskAdd, 2, 3);

    std::function<void()> taskAdd_2 = std::bind(taskAdd, 29, 3);
    pool.AddTask(taskAdd_2);

    Worker w;
    pool.AddTask(&Worker::task, &w, 2, 3);
    pool.stop();
  }

  {
    ThreadPool pool(4);
    pool.start();
    pool.AddTask(taskAdd, 4, 3);

    std::function<void()> taskAdd_2 = std::bind(taskAdd, 29, 3);
    pool.AddTask(taskAdd_2);

    Worker w;
    pool.AddTask(&Worker::task, &w, 2, 3);
    pool.join();
    EXPECT_EQ(pool.tasksRemaining(), 0);
    pool.stop();
  }
  // ThreadPool::Task func = std::move([]() { taskAdd(2, 3); });
  // pool.AddTask(func);

  //  auto result = pool.AddTask([](int answer) { return answer; }, 42);
  // std::cout << result.get() << std::endl;
}

TEST(test_ThreadPool, AddTaskSync) {
  {
    ThreadPool pool(1);
    pool.start();
    int ret = pool.AddTaskSync(taskAdd2, 1, 2, 3);
    ASSERT_EQ(0, ret);

    pool.stop();
  }
}
