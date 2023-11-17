#ifndef KINGFISHER_BASE_NET_HTTP_INTERCEPTOR_H_
#define KINGFISHER_BASE_NET_HTTP_INTERCEPTOR_H_

// #include "chain_interceptor.h"

namespace kingfisher {
namespace net {

class HttpChainInterceptor;

class HttpInterceptor {
 public:
  HttpInterceptor() {}
  virtual ~HttpInterceptor() {}
  virtual int Intercept(HttpChainInterceptor &chain) = 0;
};

using HttpInterceptors = std::vector<std::shared_ptr<HttpInterceptor>>;

}  // namespace net
}  // namespace kingfisher

#endif
