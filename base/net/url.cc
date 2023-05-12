#include "url.h"
#include "crypto/hex.h"

namespace kingfisher {
namespace net {

std::string URLEncode(const std::string &source) {
  std::ostringstream result;
  for (auto i = source.begin(); i != source.end(); ++i) {
    if (isalnum(*i) || *i == '-' || *i == '_' || *i == '.' || *i == '~') {
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

std::string URLDecode(const std::string &source) {
  std::ostringstream result;
  for (size_t i = 0; i < source.size(); ++i) {
    if (source[i] == '%') {
      if (i < source.size() - 2) {
        if (source[i + 1] && source[i + 2]) {
          result << static_cast<char>(
                        (kingfisher::crypto::FromHex(source[i + 1]) << 4 |
                         kingfisher::crypto::FromHex(source[i + 2])));
          i += 2;
        }
      }
    } else if (source[i] == '+') {
      result << ' ';
    } else {
      result << source[i];
    }
  }

  return result.str();
}
}
}
