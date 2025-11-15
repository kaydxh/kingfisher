#ifndef KINGFISHER_BASE_NET_HTTP_CURL_CLIENT_H_
#define KINGFISHER_BASE_NET_HTTP_CURL_CLIENT_H_

#include <memory>
#include <mutex>

#include "container/buffer.h"
#include <curl/curl.h>
#include "net/http/chain_interceptor.h"
#include "net/http/interceptor.h"

namespace kingfisher {
namespace net {

class CurlClient : public HttpInterceptor {
 public:
  CurlClient();
  ~CurlClient();

  int Init();
  int Intercept(HttpChainInterceptor &chain);

 private:
  static size_t writeCallback(char *ptr, size_t size, size_t nmemb,
                              void *userdata);
  static size_t readCallback(char *ptr, size_t size, size_t nmemb,
                             void *userdata);

  // 自定义删除器curl_slice_free_all
  std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> GetHeaders()
      const;

 private:
  CURL *curl_ = nullptr;
  // std::string reader_buffer_;
  HttpChainInterceptor *chain_;
  std::unique_ptr<container::Buffer> reader_buffer_;
  static std::once_flag once_;
  std::string error_message_;
};

}  // namespace net
}  // namespace kingfisher

#endif
