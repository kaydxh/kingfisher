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
  FaceDetectPool(const CoreThreadPoolOptions& opts,
                 const std::string& model_path) {
    int ret = FaceDetectSDK::GlobalInit(model_path, opts.max_batch_size);
    if (ret != 0) {
      std::cout << "FaceDetectSDK GlobalInit failed" << std::endl;
      exit(1);
    }
    pl = new kingfisher::pool::CoreThreadPool<FaceDetectSDK, FaceDetectTask>(
        opts, Process);
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
  std::cout << "face_detect_task->ret_ = " << face_detect_task->ret_
            << std::endl;
}
