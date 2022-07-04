#ifndef KINGFISHER_PKG_CV_WRAP_FUNC_H_
#define KINGFISHER_PKG_CV_WRAP_FUNC_H_

#include "Magick++.h"

namespace kingfisher {
namespace kcv {

// Wrap call to f (args), try catch exception
// https://stackoverflow.com/questions/51451843/creating-a-template-to-wrap-c-member-functions-and-expose-as-c-callbacks
template <typename Func, typename... Args>
ssize_t WrapFuncT(Func f, Args... args) {
  try {
    f(args...);
  } catch (Magick::Warning &w) {
    std::cout << "warn: " << w.what() << std::endl;
  } catch (Magick::Error &e) {
    std::cout << "a Magick++ error occurred: " << e.what() << std::endl;
    return -1;
  } catch (...) {
    std::cout << "an unhandled error has occurred; exiting application."
              << std::endl;
    return -1;
  }

  return 0;
}
}  // namespace kcv
}  // namespace kingfisher

#endif
