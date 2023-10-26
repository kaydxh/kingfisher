#include "random.h"

namespace kingfisher {
namespace random {

// https://github.com/sliftist/rdtsc-now/blob/main/binding.cc
#if defined(__linux__)
#if defined(__x86_64__) || defined(__aarch64__)
__inline__ uint64_t rdtsc() {
  uint64_t low, high;
  __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
  return (high << 32) | low;
}
#elif defined(__i386__)
__inline__ uint64_t rdtsc() {
  uint64_t ret;
  __asm__ volatile("rdtsc" : "=A"(ret));
  return ret;
}

#endif
#endif

static __thread RandomSeed seed_thread_safe = {false, 0};

void Random::initRandomSeed() {
  seed_thread_safe.init = true;
  seed_thread_safe.seed = rdtsc();
}

void Random::initRandomSeedAtFork() {
  pthread_atfork(initRandomSeed, initRandomSeed, initRandomSeed);
}

void Random::initRandomSeedOnce() {
  static pthread_once_t once = PTHREAD_ONCE_INIT;
  pthread_once(&once, initRandomSeedAtFork);
  initRandomSeed();
}

unsigned int Random::GetRandom() {
  if (!seed_thread_safe.init) {
    initRandomSeedOnce();
  }
  return rand_r(&seed_thread_safe.seed);
}

unsigned int Random::GetRandomRange(unsigned int min_val,
                                    unsigned int max_val) {
  if (min_val == max_val) {
    return max_val;
  } else if (max_val < min_val) {
    unsigned int tmp = min_val;
    min_val = max_val;
    max_val = tmp;
  }

  unsigned int n = GetRandom() % (max_val - min_val);
  return n + min_val;
}

float Random::GetRandomPercentage() { return GetRandomRange(0, 100) / 100.0; }

}  // namespace random
}  // namespace kingfisher
