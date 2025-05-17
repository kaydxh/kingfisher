#include "match.h"

#include <glob.h>
#include <memory.h>
#include <sys/stat.h>  // 主要头文件
#include <sys/types.h> // 某些系统可能需要额外包含（如旧版 Linux）

#include <sstream>

namespace kingfisher {
namespace path {

int  Glob(const std::string& pattern, std::vector<std::string>& matches) {
  glob_t glob_result;
  memset(&glob_result, 0, sizeof(glob_result));

  int ret = glob(pattern.c_str(), GLOB_TILDE | GLOB_MARK, NULL, &glob_result);
  if (ret != 0 && ret != GLOB_NOMATCH) {
    globfree(&glob_result);
    std::stringstream ss;
    ss << "glob() failed, err: " << ret << std::endl;
    return ret;
  }

  for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
    std::string path = glob_result.gl_pathv[i];
    matches.push_back(path);
  }

  globfree(&glob_result);

  return 0;
}

}  // namespace path
}  // namespace kingfisher
