#include <gtest/gtest.h>

#include <memory>

#include "log/config.h"
#include "mysql-v6.1.11/include/binary_log_types.h"
#include "pool/task/pool.hpp"
#include "pool/task/task.hpp"

namespace sdk {

class test_Sdk_Pool : public testing::Test {
   public:
    test_Sdk_Pool() {}
    ~test_Sdk_Pool() {}

    virtual void SetUp(void) {}

    virtual void TearDown(void) {}
};

int GlobalInit() {
    LOG(INFO) << "do global init";
    return 0;
}

int GlobalRelease() {
    LOG(INFO) << "do global release";
    return 0;
}

int LocalInit(void* p, int id) {
    LOG(INFO) << "do local init id: " << id;
    return 0;
}

void LocalRelease(void* p) { LOG(INFO) << "do local release"; }

class MySDK {
   public:
    MySDK() {}
    ~MySDK() {}
    int doSdk() {
        LOG(INFO) << "do in sdk";
        return 0;
    }
};

struct MyTask : kingfisher::pool::Task {
    int ret;
};

void Process(MySDK&, std::vector<std::shared_ptr<MyTask>>& task) {
    LOG(INFO) << "process task";
}

TEST(test_Sdk_Pool, All) {
    kingfisher::pool::CoreThreadPoolOptions opts;
    opts.global_init_func = GlobalInit;
    opts.global_release_func = GlobalRelease;
    opts.local_init_func = LocalInit;
    opts.core_ids = {-1};

    kingfisher::pool::CoreThreadPool<MySDK, MyTask> pool(opts);
    int ret = pool.Init(Process);
    if (ret != 0) {
        return;
    }

    std::shared_ptr<MyTask> task = std::make_shared<MyTask>();
    LOG(INFO) << "commit task";
    pool.Commit(task);
    LOG(INFO) << "wait task";
    task->Wait();
    LOG(INFO) << "task done";
    sleep(10000);
}

}  // namespace sdk
