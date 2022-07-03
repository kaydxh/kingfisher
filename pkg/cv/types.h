#ifndef KINGFISHER_PKG_CV_TYPES_H_
#define KINGFISHER_PKG_CV_TYPES_H_
namespace kingfisher {
namespace kcv {

enum ColorSpace {
  UnknownColorSpace = 0,
  BGRColorSpace = 1,
  BGRAColorSpace = 2,
  GRAYColorSpace = 3,
  GRAYAColorSpace = 4
};

struct Rect {
  int x;
  int y;
  int height;
  int width;
};

}  // namespace kcv
}  // namespace kingfisher

#endif
