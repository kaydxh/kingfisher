#ifndef KINGFISHER_BASE_PATH_MATCH_H_
#define KINGFISHER_BASE_PATH_MATCH_H_

#include <string>
#include <vector>

namespace kingfisher {
namespace path {
int  Glob(const std::string& pattern, std::vector<std::string>& matches);
}
}  // namespace kingfisher

#endif
