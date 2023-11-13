#include "pool_instance.h"

#include <memory>

#include "log/config.h"
#include "strings/strings.h"
#include "time/time_counter.h"

namespace kingfisher {
namespace pool {

// thread pool size is 1, means CoreInstanceHolder run in one thread
CoreInstanceHolder::CoreInstanceHolder() : thread_pool_(1) {
  thread_pool_.start();
}

CoreInstanceHolder::~CoreInstanceHolder() { thread_pool_.stop(); }

void CoreInstanceHolder::Do(std::function<int()> f) {
  thread_pool_.AddTaskSync(f, 0);
}

int Pool::init() {
  for (const auto& id : opts_.core_ids) {
    for (int64_t i = 0; i < opts_.reserve_pool_size_per_core; ++i) {
      auto holder = std::make_shared<CoreInstanceHolder>();
      holder->name = opts_.name;
      holder->core_id = id;
      holder->model_paths = opts_.model_paths;
      holder->batch_size = opts_.batch_size;

      holder->Do([&]() -> int {
        holder->instance = new_func_();
        return 0;
      });

      if (opts_.local_init_func) {
        opts_.local_init_func(holder->instance);
      }
      holders_[id].push(holder);
      LOG(INFO) << strings::FormatString("put core id: %d, holder: %p", id,
                                         holder->instance);
    }
  }
  return 0;
}

int Pool::GlobalInit() {
  if (opts_.global_init_func) {
    auto ret = opts_.global_init_func();
    if (ret != 0) {
      return ret;
    }
  }

  return init();
}

int Pool::GlobalRelease() {
  for (auto it = holders_.begin(); it != holders_.end(); ++it) {
    for (unsigned int i = 0; i < it->second.size(); ++i) {
      auto holder = it->second.front();
      holder->Do([&]() -> int {
        if (opts_.local_release_func) {
          opts_.local_release_func(holder.get());
        }
        if (opts_.delete_func) {
          opts_.delete_func(holder.get());
        }
        return 0;
      });
      it->second.pop();
    }
  }

  return 0;
}

int Pool::GetByCoreId(std::shared_ptr<CoreInstanceHolder>& holder,
                      int64_t core_id) {
  if (opts_.capacity_pool_size_per_core <= 0) {
    return -1;
  }

  std::unique_lock<std::mutex> lock(holder_mutex_);
  if (!holder_cv_.wait_for(lock,
                           std::chrono::milliseconds(opts_.wait_timeout_once),
                           [&]() { return !holders_[core_id].empty(); })) {
    LOG(ERROR) << strings::FormatString(
        "wait for getting holder timeout %dms for core id %d, size: %d",
        opts_.wait_timeout_once, core_id, holders_[core_id].size());
    return -1;
  }

  if (holders_[core_id].empty()) {
    LOG(ERROR) << strings::FormatString(
        "core id: %d has not avaliable instance", core_id);
    return -1;
  }

  holder = holders_[core_id].front();
  holders_[core_id].pop();
  LOG(INFO) << strings::FormatString("get core id: %d, holder: %p", core_id,
                                     holder->instance);

  return 0;
}

int Pool::GetWithRoundRobinMode(std::shared_ptr<CoreInstanceHolder>& holder) {
  auto remain = opts_.wait_timeout_total;

  while (true) {
    for (const auto& id : opts_.core_ids) {
      time::TimeCounter tc;

      int ret = GetByCoreId(holder, id);
      if (ret == 0) {
        return ret;
      }

      if (opts_.wait_timeout_total > 0) {
        remain -= tc.Elapse();
        if (remain <= 0) {
          return -1;
        }
      }
    }
  }
}

int Pool::Get(std::shared_ptr<CoreInstanceHolder>& holder) {
  switch (opts_.load_balance_mode) {
    case RoundRobinBalanceMode:
      return GetWithRoundRobinMode(holder);
    default:
      return -1;
  }
}

void Pool::Put(const std::shared_ptr<CoreInstanceHolder>& holder) {
  std::unique_lock<std::mutex> lock(holder_mutex_);
  holders_[holder->core_id].push(holder);
  holder_cv_.notify_one();
  LOG(INFO) << strings::FormatString("put core id: %d, holder: %p",
                                     holder->core_id, holder->instance);
}

}  // namespace pool
}  // namespace kingfisher
