#include "hex.h"
namespace kingfisher {
namespace crypto {

char ToHex(char c) {
  static const char hex[] = "0123456789abcdef";
  return hex[c & 0xf];
}

char FromHex(char c) { return isdigit(c) ? c - '0' : tolower(c) - 'a' + 10; }
}
}
