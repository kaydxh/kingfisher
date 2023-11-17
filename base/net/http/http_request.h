#ifndef KINGFISHER_BASE_NET_HTTP_HTTP_REQUEST_H_
#define KINGFISHER_BASE_NET_HTTP_HTTP_REQUEST_H_

#include <map>
#include <string>

namespace kingfisher {
namespace net {

enum class HttpMethod { GET, POST, PUT, OPTIONS, HEAD, PATCH, DELETE };

const std::string HTTP_METHOD_GET = "GET";
const std::string HTTP_METHOD_POST = "POST";

struct KeyCmp {
  bool operator()(const std::string& s1, const std::string& s2) const {
    return std::lexicographical_compare(
        s1.begin(), s1.end(), s2.begin(), s2.end(),
        [](char c1, char c2) { return ::tolower(c1) < ::tolower(c2); });
  }
};

using Headers = std::multimap<std::string, std::string, KeyCmp>;

class HttpRequest {
 public:
  const std::string& Url() { return url_; }
  void SetUrl(const std::string& url) { url_ = url; }

  std::string Method() { return method_; }
  void SetMethod(const std::string& method) { method_ = method; }

  const std::string& Body() { return body_; }
  void SetBody(const std::string& body) { body_ = body; }

 private:
  std::string url_;
  std::string method_;
  Headers headers_;
  std::string body_;

  int64_t timeout_ms_ = 5000;
};

}  // namespace net
}  // namespace kingfisher

#endif
