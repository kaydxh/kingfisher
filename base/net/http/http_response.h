#ifndef KINGFISHER_BASE_NET_HTTP_HTTP_RESPONSE_H_
#define KINGFISHER_BASE_NET_HTTP_HTTP_RESPONSE_H_

#include <string>

namespace kingfisher {
namespace net {

class HttpResponse {
 public:
  int StatusCode() { return status_code_; }
  void SetStatusCode(int status_code) { status_code_ = status_code; }

  const std::string& Body() { return body_; }
  void SetBody(const std::string& body) { body_ = body; }

 private:
  int status_code_ = 0;
  std::string body_;
};

}  // namespace net
}  // namespace kingfisher

#endif
