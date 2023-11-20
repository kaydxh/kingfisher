#ifndef KINGFISHER_BASE_NET_HTTP_HTTP_REQUEST_H_
#define KINGFISHER_BASE_NET_HTTP_HTTP_REQUEST_H_

#include <map>
#include <string>

#include "http_headers.h"

namespace kingfisher {
namespace net {

class HttpRequest {
 public:
  const std::string& Url() { return url_; }
  void SetUrl(const std::string& url) { url_ = url; }

  std::string Method() { return method_; }
  void SetMethod(const std::string& method) { method_ = method; }

  const std::string& Body() { return body_; }
  void SetBody(const std::string& body) { body_ = body; }

  HttpHeaders& Headers() { return headers_; }

 private:
  std::string url_;
  std::string method_;
  HttpHeaders headers_;
  std::string body_;
};

}  // namespace net
}  // namespace kingfisher

#endif
