#include "curl_client.h"

#include <curl/easy.h>

#include "log/config.h"

namespace kingfisher {
namespace net {

std::once_flag CurlClient::once_;

#define XSET_EASY_OPT(curl, opt, val)                           \
  {                                                             \
    CURLcode ret = curl_easy_setopt(curl, opt, val);            \
    if (ret != CURLE_OK) {                                      \
      LOG(ERROR) << "failed to curl_easy_setopt, ret: " << ret; \
      return ret;                                               \
    }                                                           \
  }

CurlClient::CurlClient() {}

CurlClient::~CurlClient() {
  if (curl_) {
    curl_easy_cleanup(curl_);
  }
}

int CurlClient::Init() {
  std::call_once(once_, []() { curl_global_init(CURL_GLOBAL_ALL); });

  curl_ = curl_easy_init();
  if (!curl_) {
    LOG(ERROR) << "failed to curl_easy_init";
    return -1;
  }

  XSET_EASY_OPT(curl_, CURLOPT_NOSIGNAL, 1L)

  return 0;
}

int CurlClient::Intercept(HttpChainInterceptor &chain) {
  if (curl_ == nullptr) {
    LOG(ERROR) << "curl is nil";
    return -1;
  }

  auto &request = chain.Request();
  XSET_EASY_OPT(curl_, CURLOPT_URL, request.Url().c_str());
  XSET_EASY_OPT(curl_, CURLOPT_CUSTOMREQUEST, request.Method().c_str());

  XSET_EASY_OPT(curl_, CURLOPT_READDATA, this);
  // XSET_EASY_OPT(curl_, CURLOPT_POST, 1L);

  auto code = curl_easy_perform(curl_);
  if (code != CURLE_OK) {
    LOG(ERROR) << "failed to curl_easy_perform, code: " << code;
    return -1;
  }
  long status = 0;
  code = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &status);
  if (code != CURLE_OK) {
    LOG(ERROR) << "failed to curl_easy_getinfo, code: " << code;
    return -1;
  }

  return chain.Handler();
}

size_t CurlClient::writeCallback(char *ptr, size_t size, size_t nmemb,
                                 void *userdata) {
  if (ptr == nullptr || userdata == nullptr) {
    return 0;
  }
  size_t len = size * nmemb;

  return len;
}

}  // namespace net
}  // namespace kingfisher
