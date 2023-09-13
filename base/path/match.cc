#include "match.h"

#include <glob.h>
#include <memory.h>

#include <sstream>

namespace kingfisher {
namespace path {

std::vector<std::string> Glob(const std::string& pattern) {
  std::vector<std::string> matches;

  glob_t glob_result;
  memset(&glob_result, 0, sizeof(glob_result));

  int ret = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
  if (ret != 0) {
    globfree(&glob_result);
    std::stringstream ss;
    ss << "glob() failed, err: " << ret << std::endl;
    throw std::runtime_error(ss.str());
  }

  for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
    matches.push_back(std::string(glob_result.gl_pathv[i]));
  }

  globfree(&glob_result);

  return matches;
}

}  // namespace path
}  // namespace kingfisher
