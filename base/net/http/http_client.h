#ifndef KINGFISHER_BASE_NET_HTTP_HTTP_CLIENT_H_
#define KINGFISHER_BASE_NET_HTTP_HTTP_CLIENT_H_

#include <memory>
#include <vector>

#include "http_request.h"
#include "http_response.h"
#include "types.h"

namespace kingfisher {
namespace net {

class HttpInterceptor;

class HttpClient {
 public:
  HttpClient();
  ~HttpClient();

  int Init();

  int Get(HttpRequest& req, HttpResponse& resp);
  int Post(HttpRequest& req, HttpResponse& resp);
  int Put(HttpRequest& req, HttpResponse& resp);
  int Do(HttpRequest& req, HttpResponse& resp);

  void SetConnectTimeoutMs(int64_t timeout) { connect_timeout_ms_ = timeout; }

  int64_t ConnectTimeoutMs() { return connect_timeout_ms_; }

  void SetTimeoutMs(int64_t timeout) { timeout_ms_ = timeout; }
  int64_t TimeoutMs() { return timeout_ms_; }

  void SetProxy(const std::string& proxy);
  std::string Proxy() { return proxy_; }

  void AddInterceptor(const std::shared_ptr<HttpInterceptor> interceptor);

 private:
  HttpInterceptors interceptors_;
  std::string proxy_;

  int64_t connect_timeout_ms_ = 5000;
  int64_t timeout_ms_ = 5000;
};

}  // namespace net
}  // namespace kingfisher

#endif
