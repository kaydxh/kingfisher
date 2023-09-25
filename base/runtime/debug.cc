#include "debug.h"

#include <unistd.h>

namespace kingfisher {
namespace runtime {
int NumCPU() { return sysconf(_SC_NPROCESSORS_ONLN); }

}  // namespace runtime
}  // namespace kingfisher
