#include "cvtable.h"

#include <sys/types.h>

#include "file/file.h"

namespace kingfisher {
namespace kcv {

CVTable::CVTable() { /*cv_table_.resize(100);*/
}

CVTable::~CVTable() {}

int CVTable::Init(const std::string& filename) {
  file::File file(filename.c_str());
  int ret = file.ReadLines<float>(cv_table_);
  if (ret != 0) {
    return ret;
  }

  if (cv_table_.size() != 100) {
    return -1;
  }

  return 0;
}

float CVTable::Score(float sim) {
  if (sim <= 0.0) {
    return 0.0;
  }

  if (sim >= 1.0) {
    return 1.0;
  }

  float score = 0.0;
  for (int i = 99; i > 0; i--) {
    if (sim >= cv_table_[i - 1]) {
      float delta = cv_table_[i] - cv_table_[i - 1];
      score = (sim - cv_table_[i - 1]) / delta + (i - 1) + 1;
      return score;
    }
  }

  return 0;
}

void CVTable::Print() {
  std::cout << "Print size: " << cv_table_.size() << std::endl;
  for (size_t i = 0; i < cv_table_.size(); ++i) {
    std::cout << i << " " << cv_table_[i] << std::endl;
  }
}

}  // namespace kcv
}  // namespace kingfisher
