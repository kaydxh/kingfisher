#ifndef KINGFISHER_BASE_NET_HTTP_HTTP_HEADERS_H_
#define KINGFISHER_BASE_NET_HTTP_HTTP_HEADERS_H_

#include <map>
#include <string>
#include <vector>

namespace kingfisher {
namespace net {

struct KeyCmp {
  bool operator()(const std::string& s1, const std::string& s2) const {
    return std::lexicographical_compare(
        s1.begin(), s1.end(), s2.begin(), s2.end(),
        [](char c1, char c2) { return ::tolower(c1) < ::tolower(c2); });
  }
};

using Headers = std::multimap<std::string, std::string, KeyCmp>;

class HttpHeaders {
 public:
  HttpHeaders();
  ~HttpHeaders();

  void SetHeader(const std::string& key, const std::string& value);
  void AppendHeader(const std::string& key, const std::string& value);

  std::string GetHeader(const std::string& key);
  std::vector<std::string> GetHeaders(const std::string& key);

  void RemoveHeaders(const std::string& key);
  void RemoveHeader(const std::string& key, const std::string& value);

  void SetContentLength(int64_t sz);

 private:
  Headers headers_;
};

}  // namespace net
}  // namespace kingfisher

#endif
