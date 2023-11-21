#include "wait.h"

namespace kingfisher {
namespace time {

int Retry(std::function<int()> f, int64_t period_ms, int retry_times) {
  if (retry_times <= 0) {
    retry_times = 1;
  }

  int ret = 0;
  for (int i = 0; i < retry_times; ++i) {
    ret = f();
    if (ret == 0) {
      break;
    }
    time::MsSleep(period_ms);
  }

  return ret;
}

}  // namespace time
}  // namespace kingfisher
