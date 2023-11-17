#include "chain_interceptor.h"

#include "interceptor.h"

namespace kingfisher {
namespace net {

int HttpChainInterceptor::Handler() {
  if (begin_ == end_) {
    return 0;
  }

  HttpChainInterceptor next_chain_interceptor(http_client_, request_, response_,
                                              begin_ + 1, end_);
  auto& interceptor = *begin_;
  return interceptor->Intercept(next_chain_interceptor);
}

}  // namespace net
}  // namespace kingfisher
