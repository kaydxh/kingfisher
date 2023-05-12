#ifndef KINGFISHER_BASE_NET_URL_H_
#define KINGFISHER_BASE_NET_URL_H_

#include <sstream>
#include <string>
#include "crypto/hex.h"

namespace kingfisher {
namespace net {

// http://www.geekhideout.com/urlcode.shtml
std::string URLEncode(const std::string &source);
std::string URLDecode(const std::string &source);
}
}

#endif
