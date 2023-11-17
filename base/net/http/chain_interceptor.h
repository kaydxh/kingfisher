#ifndef KINGFISHER_BASE_NET_HTTP_CHAIN_INTERCEPTOR_H_
#define KINGFISHER_BASE_NET_HTTP_CHAIN_INTERCEPTOR_H_

#include <memory>
#include <vector>

#include "http_client.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"

namespace kingfisher {
namespace net {

class HttpInterceptor;
using HttpInterceptors = std::vector<std::shared_ptr<HttpInterceptor>>;

class HttpChainInterceptor {
 public:
  HttpChainInterceptor(HttpClient& client, HttpRequest& request,
                       HttpResponse& response, HttpInterceptors::iterator begin,
                       HttpInterceptors::iterator end)
      : http_client_(client),
        request_(request),
        response_(response),
        begin_(begin),
        end_(end) {}

  int Handler();

  HttpRequest& Request() { return request_; }

 private:
  HttpClient http_client_;
  HttpRequest request_;
  HttpResponse response_;
  HttpInterceptors::iterator begin_;
  HttpInterceptors::iterator end_;
};

}  // namespace net
}  // namespace kingfisher

#endif
