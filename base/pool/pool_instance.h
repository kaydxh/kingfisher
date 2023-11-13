#ifndef BASE_POOL_POOL_INSTANCE_H_
#define BASE_POOL_POOL_INSTANCE_H_

#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "core/scope_guard.h"
#include "log/config.h"
#include "thread/thread_pool.h"

namespace kingfisher {
namespace pool {

class CoreInstanceHolder {
 public:
  CoreInstanceHolder();
  ~CoreInstanceHolder();

  void Do(std::function<int()> f);

  void* instance = nullptr;
  std::string name;
  int64_t core_id = 0;
  std::vector<std::string> model_paths;
  int64_t batch_size = 1;
  thread::ThreadPool thread_pool_;
};

enum LoadBalanceMode {
  RoundRobinBalanceMode = 0,
};

struct PoolOptions {
  std::string name;
  int64_t reserve_pool_size_per_core = 0;
  int64_t capacity_pool_size_per_core = 1;
  int64_t idle_timeout = 10000;  // 10s
  int64_t wait_timeout_once = 100;
  int64_t wait_timeout_total = 1000;
  std::vector<int64_t> core_ids;
  std::vector<std::string> model_paths;
  int64_t batch_size = 0;

  LoadBalanceMode load_balance_mode = RoundRobinBalanceMode;

  std::function<int()> global_init_func;
  std::function<int()> global_release_func;
  std::function<void(void*)> local_init_func;
  std::function<void(void*)> local_release_func;
  std::function<void(void*)> delete_func;
};

class Pool {
 public:
  using NewFunc = std::function<void*()>;

  Pool(NewFunc nf, const PoolOptions& options)
      : new_func_(nf), opts_(options) {}

  int GlobalInit();
  int GlobalRelease();
  int Get(std::shared_ptr<CoreInstanceHolder>& holder);
  int GetByCoreId(std::shared_ptr<CoreInstanceHolder>& holder, int64_t core_id);
  int GetWithRoundRobinMode(std::shared_ptr<CoreInstanceHolder>& holder);
  void Put(const std::shared_ptr<CoreInstanceHolder>& holder);

  template <typename F>
  void Invoke(F&& f) {
    std::shared_ptr<CoreInstanceHolder> holder;
    int ret = Get(holder);
    if (ret != 0) {
      LOG(ERROR) << "failed to get holder, ret: " << ret;
      return;
    }
    SCOPE_EXIT { Put(holder); };

    holder->Do([&]() -> int {
      f(holder->instance);
      return 0;
    });

    return;
  }

 private:
  int init();

 private:
  NewFunc new_func_;
  PoolOptions opts_;
  std::map<int64_t, std::queue<std::shared_ptr<CoreInstanceHolder>>> holders_;
  std::mutex holder_mutex_;
  std::condition_variable holder_cv_;
};

}  // namespace pool
}  // namespace kingfisher

#endif
