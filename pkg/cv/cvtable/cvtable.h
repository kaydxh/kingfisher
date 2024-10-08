#ifndef KINGFISHER_PKG_CV_CVTABLE_H_
#define KINGFISHER_PKG_CV_CVTABLE_H_

#include <string>
#include <vector>

namespace kingfisher {
namespace kcv {

class CVTable {
 public:
  CVTable();
  ~CVTable();

  int Init(const std::string& file);
  float Sim(float score);
  float Score(float sim);

  void Print();

 private:
  std::vector<float> cv_table_;
};

}  // namespace kcv
}  // namespace kingfisher

#endif
