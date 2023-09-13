#ifndef KINGFISHER_BASE_PATH_MATCH_H_
#define KINGFISHER_BASE_PATH_MATCH_H_

#include <string>
#include <vector>

namespace kingfisher {
namespace path {

std::vector<std::string> Glob(const std::string &pattern);

}
}  // namespace kingfisher

#endif
