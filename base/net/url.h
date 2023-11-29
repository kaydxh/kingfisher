#ifndef KINGFISHER_BASE_NET_URL_H_
#define KINGFISHER_BASE_NET_URL_H_

#include <map>
#include <sstream>
#include <string>

#include "crypto/hex.h"

namespace kingfisher {
namespace net {

// http://www.geekhideout.com/urlcode.shtml
std::string URLEncode(const std::string &source);
std::string URLDecode(const std::string &source);

class URI {
 public:
  struct QueryParam {
    std::string key;
    std::string value;

    bool operator==(const QueryParam &r) const {
      return key == r.key && value == r.value;
    }

    bool operator<(const QueryParam &r) const {
      int c = key.compare(r.key);
      if (c != 0) {
        return c < 0;
      }
      return value < r.value;
    }
  };

 public:
  URI();
  ~URI();

  const std::string &Scheme() const { return scheme_; }
  const std::string &Path() const { return path_; }

  const std::map<std::string, std::string> &QueryParams() const {
    return query_params_;
  }

  static int Parse(URI &uri, const std::string &url);

  std::string PercentDecode(const std::string &s);

 private:
  std::string scheme_;
  std::string path_;
  std::map<std::string, std::string> query_params_;
};

}  // namespace net
}  // namespace kingfisher

#endif
