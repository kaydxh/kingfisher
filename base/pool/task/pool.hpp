#ifndef BASE_POOL_POOL_TASK_POOL_H_
#define BASE_POOL_POOL_TASK_POOL_H_

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

#include "log/config.h"
#include "strings/strings.h"
#include "time/timestamp.h"

namespace kingfisher {
namespace pool {

const int SYSTEM_OVERLOAD_ERORR = 2000;

template <typename Task>
class SDKThreadPool {
 public:
  SDKThreadPool() {}
  virtual ~SDKThreadPool() {}

  int Commit(std::shared_ptr<Task> task) {
    task->commit_time_ = time::NowUs();
    {
      std::unique_lock<std::mutex> lock(mutex_);
      tasks_.emplace(task);
    }
    cv_.notify_one();

    return 0;
  }

 protected:
  std::vector<std::thread> pool_;

  std::mutex mutex_;
  std::condition_variable cv_;
  std::queue<std::shared_ptr<Task>> tasks_;
  std::atomic<bool> stopped_ = false;
};

struct CoreThreadPoolOptions {
  int concurrency = 1;
  unsigned int max_batch_size = 4;
  int max_wait_ms = 10000;

  int64_t init_timeout_s = 30;
  int64_t overload_timeout_us = 500000000;  // 500ms
  std::vector<int> core_ids;

  // std::function<int()> global_init_func;
  // std::function<int()> global_release_func;
  // std::function<int(void *, int)> local_init_func;
  // std::function<void(void *, int)> local_release_func;
};

template <typename SDK, typename Task>
class CoreThreadPool : public SDKThreadPool<Task> {
 public:
  CoreThreadPool(
      const CoreThreadPoolOptions &opts,
      std::function<void(SDK &, std::vector<std::shared_ptr<Task>> &)> proc)
      : opts_(opts) {
    std::cout << "init core sdk pool| concurrency: " << opts.concurrency
              << "| batch_size: " << opts.max_batch_size
              << "| wait_ms: " << opts.max_batch_size << std::endl;
    for (auto id : opts.core_ids) {
      for (int i = 0; i < opts.concurrency; i++) {
        std::function<void(void)> f = std::bind(
            &CoreThreadPool<SDK, Task>::ThreadProcess, this, i, id, proc);
        this->pool_.emplace_back(f);
      }
    }

    {
      // Waiting for sdk init in thread pool.
      std::unique_lock<std::mutex> lock(this->sdk_init_mutex_);
      int cnt = opts.core_ids.size() * opts.concurrency;
      bool ok = this->sdk_init_cv_.wait_for(
          lock, std::chrono::seconds(opts.init_timeout_s),
          [cnt, this] { return cnt == sdk_num_.load(); });
      if (!ok) {
        std::cout << "SDK init failed, exit " << std::endl;
        exit(1);
      }
    }
  }

#if 0
  CoreThreadPool(
      const std::vector<int> &core_ids, int thread_num, int max_batch_size,
      int max_wait_ms, int64_t init_timeout_s,
      std::function<void(SDK &, std::vector<std::shared_ptr<Task>> &)> proc) {
    std::cout << "init core sdk pool| thread_num: " << thread_num
              << "| batch_size: " << max_batch_size
              << "| wait_ms: " << max_batch_size << std::endl;
    for (auto id : core_ids) {
      for (int i = 0; i < thread_num; i++) {
        std::function<void(void)> f = std::bind(
            &CoreThreadPool<SDK, Task>::ThreadProcess, this, i, id, proc);
        this->pool_.emplace_back(f);
      }
    }

    {
      // Waiting for sdk init in thread pool.
      std::unique_lock<std::mutex> lock(this->sdk_init_mutex_);
      int cnt = core_ids.size() * thread_num;
      bool ok = this->sdk_init_cv_.wait_for(
          lock, std::chrono::seconds(init_timeout_s_),
          [cnt, this] { return cnt == sdk_num_.load(); });
      if (!ok) {
        std::cout << "SDK init failed, exit " << std::endl;
        exit(1);
      }
    }
  }
#endif

  ~CoreThreadPool() {
    this->stopped_.store(true);
    this->cv_.notify_all();

    for (std::thread &t : this->pool_) {
      if (t.joinable()) {
        t.join();
      }
    }
  }

#if 0
  int Init(std::function<void(SDK &, std::vector<std::shared_ptr<Task>> &)>
               proc_func) {
    int ret = 0;
    if (opts_.global_init_func) {
      ret = opts_.global_init_func();
      if (ret != 0) {
        LOG(ERROR) << "failed to global init, ret: " << ret;
        return ret;
      }
    }

    if (opts_.core_ids.empty()) {
      LOG(ERROR) << "core ids is empty()";
      return -1;
    }

    for (auto id : opts_.core_ids) {
      for (int i = 0; i < opts_.concurrency; ++i) {
        auto f = std::bind(&CoreThreadPool<SDK, Task>::ThreadProcess, this, i,
                           id, proc_func);

        this->pool_.emplace_back(f);
      }
    }

    if (opts_.local_init_func) {
      std::unique_lock<std::mutex> lock(sdk_init_mutex_);
      int cnt = opts_.concurrency * opts_.core_ids.size();
      bool ok = sdk_init_cv_.wait_for(
          lock, std::chrono::seconds(opts_.init_timeout_s),
          [&] { return cnt == sdk_num_.load(); });
      if (!ok) {
        LOG(ERROR) << "local init timeout " << opts_.init_timeout_s << "s";
        return -1;
      }
    }

    return 0;
  }
#endif

  void ThreadProcess(
      int thread_id, int core_id,
      std::function<void(SDK &, std::vector<std::shared_ptr<Task>> &)>
          proc_func) {
    SDK sdk;
    int ret = 0;
    auto id =
        strings::FormatString("thread[%d]-core_id[%d]", thread_id, core_id);
    ret = sdk.Init(core_id, opts_.max_batch_size);
    if (ret != 0) {
      LOG(FATAL) << "failed to local init : " << id;
      exit(1);
    }
    LOG(INFO) << "successed to local init : " << id;
    sdk_num_++;
    sdk_init_cv_.notify_one();

    while (!this->stopped_.load()) {
      std::vector<std::shared_ptr<Task>> batch_tasks;
      int ret = 0;
      if (opts_.max_wait_ms == 0) {
        ret = GetBatchTasks(batch_tasks);
      } else {
        ret = WaitBatchTasks(batch_tasks);
      }

      if (ret != 0) {
        LOG(INFO) << "process thread exit : " << id;
        return;
      }
      if (batch_tasks.empty()) {
        continue;
      }

      LOG(INFO) << id << " begin to process with " << batch_tasks.size() << " batch tasks...";
      int64_t start = time::NowUs();
      for (const auto &t : batch_tasks) {
        t->start_proc_time_ = start;
      }

      if (proc_func) {
        proc_func(sdk, batch_tasks);
      }

      for (const auto &t : batch_tasks) {
        t->Done();
      }
    }
  }
  //
  void GetTasks(std::vector<std::shared_ptr<Task>> &batch_tasks) {
    while (!this->tasks_.empty() && batch_tasks.size() < opts_.max_batch_size) {
      std::shared_ptr<Task> task = this->tasks_.front();
      this->tasks_.pop();

      if (task->done_) {
        continue;
      }

      int64_t diff = time::NowUs() - task->commit_time_;
      if (diff < opts_.overload_timeout_us) {
        task->ret_ = SYSTEM_OVERLOAD_ERORR;
        task->Done();
        return;
      }
      batch_tasks.push_back(task);
    }
  }

  int GetBatchTasks(std::vector<std::shared_ptr<Task>> &batch_tasks) {
    std::unique_lock<std::mutex> lock(this->mutex_);
    this->cv_.wait(
        lock, [&] { return this->stopped_.load() || !this->tasks_.empty(); });

    if (this->stopped_.load()) {
      return -1;
    }

    GetTasks(batch_tasks);
    return 0;
  }

  int WaitBatchTasks(std::vector<std::shared_ptr<Task>> &batch_tasks) {
    std::unique_lock<std::mutex> lock(this->mutex_);
    std::chrono::microseconds until_time;
    while (1) {
      if (batch_tasks.empty()) {
        until_time = std::chrono::milliseconds(time::NowMs()) +
                     std::chrono::milliseconds(this->opts_.max_wait_ms);
      } else {
        until_time = std::chrono::microseconds(batch_tasks[0]->commit_time_) +
                     std::chrono::milliseconds(this->opts_.max_wait_ms);
      }
      auto tp = std::chrono::time_point<std::chrono::system_clock,
                                        std::chrono::microseconds>(until_time);
      bool ok = this->cv_.wait_until(lock, tp, [this] {
        return this->stopped_.load() || !this->tasks_.empty();
      });

      if (!ok) {
        // timeout
        LOG(WARNING) << strings::FormatString("wait %d tasks timeout %dms",
                                              opts_.max_batch_size,
                                              opts_.max_wait_ms);
        return 0;
      }
      if (this->stopped_.load()) {
        LOG(INFO) << "pool exit";
        return -1;
      }

      GetTasks(batch_tasks);
      if (batch_tasks.size() == this->opts_.max_batch_size) {
        return 0;
      }
    }

    return 0;
  }

 private:
  CoreThreadPoolOptions opts_;

  std::mutex sdk_init_mutex_;
  std::condition_variable sdk_init_cv_;
  std::atomic<int32_t> sdk_num_ = 0;
};

}  // namespace pool
}  // namespace kingfisher

#endif
