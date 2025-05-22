#include <gtest/gtest.h>
#include <pool/pool_instance.h>

#include "core/singleton.hpp"
#include "log/config.h"
#include "pool/task/pool.hpp"
#include "pool/task/task.hpp"
#include "protobuf/pb.map.h"

using namespace kingfisher::pool;

class test_CoreThreadPool : public testing::Test {
 public:
  test_CoreThreadPool() {}
  ~test_CoreThreadPool() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

#if 0
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

#endif

class FaceDetectSDK {
 public:
  FaceDetectSDK() {}
  static int GlobalInit(const std::string& model_dir, int max_batch_size) {
    std::cout << "do global init" << std::endl;
    return 0;
  }

  static void GlobalRelease() {
    std::cout << "do global release" << std::endl;
    return;
  }
  static std::string GetVersion() {
    std::cout << "do global get version" << std::endl;
    return "v1.0.0";
  }

  int Init(int gpu_id, int max_batch_size) {
    std::cout << "do init" << std::endl;
    // sdk_.reset(new face_detect());
    // sdk_->Init(gpu_id);
    return 0;
  }

  int FaceDetect(int input_i, int& result) {
    std::cout << "do face detect" << std::endl;
    result = input_i + 1;
    return 0;
  }
  // std::unique_ptr<face_detect> sdk_;
};

struct FaceDetectTask : public kingfisher::pool::SDKTask {
 public:
  FaceDetectTask(const std::string& session_id)
      : kingfisher::pool::SDKTask("FaceDetectTask", session_id) {}
  // input
  int input_ = 0;
  // output
  int result_ = 0;
};

struct FaceDetectPool {
 public:
#if 0
  FaceDetectPool(const std::vector<int>& core_ids,
                 const std::string& model_path, int thread_num,
                 int max_batch_size, int max_wait_ms) {
    int ret = FaceDetectSDK::GlobalInit(model_path, max_batch_size);
    if (ret != 0) {
      std::cout << "FaceDetectSDK GlobalInit failed" << std::endl;
      exit(1);
    }

    pl = new kingfisher::pool::CoreThreadPool<FaceDetectSDK, FaceDetectTask>(core_ids, thread_num, max_batch_size, max_wait_ms, Process);
  }
#endif

  FaceDetectPool(const CoreThreadPoolOptions& opts, const std::string& model_path) {
    int ret = FaceDetectSDK::GlobalInit(model_path, opts.max_batch_size);
    if (ret != 0) {
      std::cout << "FaceDetectSDK GlobalInit failed" << std::endl;
      exit(1);
    }
    pl = new kingfisher::pool::CoreThreadPool<FaceDetectSDK, FaceDetectTask>(opts, Process);
  }



  static void Process(FaceDetectSDK&,
                      std::vector<std::shared_ptr<FaceDetectTask>>&);
  kingfisher::pool::SDKThreadPool<FaceDetectTask>* pl;
};

struct FaceDetectConfig {
  std::vector<int> core_ids;
  std::string model_path;
  int thread_num;
  int max_batch_size;
  int max_wait_ms;
};

kingfisher::pool::SDKThreadPool<FaceDetectTask>* GetFaceDetectPool(
    const FaceDetectConfig& cfg) {
  CoreThreadPoolOptions opts;
  opts.core_ids = cfg.core_ids;
  opts.max_batch_size = cfg.max_batch_size;
  opts.max_wait_ms = cfg.max_wait_ms;
  opts.concurrency = cfg.thread_num;

  FaceDetectPool p = kingfisher::core::Singleton<FaceDetectPool>::Instance(
      opts, cfg.model_path);
  return p.pl;
}

void FaceDetectPool::Process(
    FaceDetectSDK& sdk, std::vector<std::shared_ptr<FaceDetectTask>>& tasks) {
  std::cout << "FaceDetectPool Process" << std::endl;
  if (tasks.empty()) {
    return;
  }

  for (const auto& task : tasks) {
    task->ret_ = sdk.FaceDetect(task->input_, task->result_);
    if (task->ret_ != 0) {
      std::cout << "FaceDetect ret_ = " << task->ret_ << std::endl;
    }
  }
}

TEST(test_CoreThreadPool, All) {
#if 0
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
#endif

  FaceDetectConfig config;
  config.core_ids.push_back(0);
  config.model_path = "./model";
  config.thread_num = 4;
  config.max_batch_size = 10;
  config.max_wait_ms = 0;
  auto face_detect_task = std::make_shared<FaceDetectTask>("session_id");
  face_detect_task->input_ = 1;
  auto face_detect_pool = GetFaceDetectPool(config);
  face_detect_pool->Commit(face_detect_task);
  face_detect_task->Wait();
  std::cout << "face_detect_task->ret_ = " << face_detect_task->ret_ << std::endl;
}
