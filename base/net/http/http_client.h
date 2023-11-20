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
  int Do(HttpRequest& req, HttpResponse& resp);

 private:
  HttpInterceptors interceptors_;
};

}  // namespace net
}  // namespace kingfisher

#endif
