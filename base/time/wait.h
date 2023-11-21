#ifndef KINNGFISHER_BASE_TIME_WAIT_H_
#define KINNGFISHER_BASE_TIME_WAIT_H_

#include <functional>

#include "time/timer.h"
namespace kingfisher {
namespace time {

int Retry(std::function<int()> f, int64_t period_ms, int retry_times = 1);

}  // namespace time
}  // namespace kingfisher

#endif
