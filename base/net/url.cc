#include "url.h"

namespace kingfisher {
namespace net {

std::string URLEncode(const std::string &source) {
  std::ostringstream result;
  for (auto i = source.begin(); i != source.end(); ++i) {
    if (isalnum(*i) || *i == '-' || *i == '_' || *i == '~') {
      result << *i;
    } else if (*i == ' ') {
      result << '+';
    } else {

      result << '%';
      result << kingfisher::crypto::ToHex(*i >> 4);
      result << kingfisher::crypto::ToHex(*i & 0xf);
    }
  }

  return result.str();
}
}
}
