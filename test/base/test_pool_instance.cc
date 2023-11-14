#include <gtest/gtest.h>

#include "log/config.h"
#include "pool/pool_instance.h"
#include "protobuf/pb.map.h"

using namespace kingfisher::pool;

class test_Pool : public testing::Test {
 public:
  test_Pool() {}
  ~test_Pool() {}

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

void LocalInit(void* p) { LOG(INFO) << "do local init"; }

void LocalRelease(void* p) { LOG(INFO) << "do local release"; }

class SDK {
 public:
  SDK() {}
  ~SDK() {}
  int doSdk() {
    LOG(INFO) << "do in sdk";
    return 0;
  }
};

void* NewSDK() {
  auto sdk = new SDK();
  return sdk;
}

void DeleteSdk(void* p) {
  auto s = reinterpret_cast<SDK*>(p);
  delete s;
}

TEST(test_Pool, All) {
  PoolOptions opt;
  opt.reserve_pool_size_per_core = 10;
  opt.core_ids = {0};
  opt.global_init_func = GlobalInit;
  opt.global_release_func = GlobalRelease;
  opt.local_init_func = LocalInit;
  opt.local_release_func = LocalRelease;

  Pool pool(NewSDK, opt);
  auto ret = pool.GlobalInit();
  ASSERT_EQ(0, ret);

  for (int i = 0; i < 10; ++i) {
    ret = pool.Invoke([&](void* p) {
      auto s = reinterpret_cast<SDK*>(p);
      s->doSdk();
    });
    ASSERT_EQ(0, ret);
  }
}
