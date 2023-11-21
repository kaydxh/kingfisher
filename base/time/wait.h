#ifndef KINNGFISHER_BASE_TIME_WAIT_H_
#define KINNGFISHER_BASE_TIME_WAIT_H_

#include <functional>

#include "time/timer.h"
namespace kingfisher {
namespace time {

int Retry(std::function<int()> f, int64_t period_ms, int retry_times = 1) {
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

#endif
