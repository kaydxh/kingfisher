#include "curl_client.h"

#include <curl/easy.h>

#include <memory>

#include "container/buffer.h"
#include "curl/curl.h"
#include "log/config.h"
#include "strings/strings.h"

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

CurlClient::CurlClient() : error_message_(CURL_ERROR_SIZE, 0) {}

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

  XSET_EASY_OPT(curl_, CURLOPT_ERRORBUFFER, error_message_.data());
  XSET_EASY_OPT(curl_, CURLOPT_NOSIGNAL, 1L)
  XSET_EASY_OPT(curl_, CURLOPT_READFUNCTION, readCallback)
  XSET_EASY_OPT(curl_, CURLOPT_READDATA, this);
  XSET_EASY_OPT(curl_, CURLOPT_WRITEFUNCTION, writeCallback)
  XSET_EASY_OPT(curl_, CURLOPT_WRITEDATA, this);

  return 0;
}

int CurlClient::Intercept(HttpChainInterceptor &chain) {
  if (curl_ == nullptr) {
    LOG(ERROR) << "curl is nil";
    return -1;
  }

  chain_ = &chain;

  auto &request = chain.Request();
  XSET_EASY_OPT(curl_, CURLOPT_URL, request.Url().c_str());
  XSET_EASY_OPT(curl_, CURLOPT_CUSTOMREQUEST, request.Method().c_str());
  XSET_EASY_OPT(curl_, CURLOPT_CONNECTTIMEOUT_MS,
                chain_->Client().ConnectTimeoutMs());
  XSET_EASY_OPT(curl_, CURLOPT_TIMEOUT_MS, chain_->Client().TimeoutMs());

  // XSET_EASY_OPT(curl_, CURLOPT_POST, 1L);
  reader_buffer_ = std::make_unique<container::Buffer>(request.Body());
  auto content_length = reader_buffer_->Size();
  if (content_length > 0) {
    // set header Content-length
    request.Headers().SetContentLength(content_length);
  }
  LOG(INFO) << "requet body size: " << reader_buffer_->Size();

  // 需要配合使用,才可进入readcallback函数
  // curl_easy_setopt(curl_, CURLOPT_UPLOAD, 1L);
  // XSET_EASY_OPT(curl_, CURLOPT_INFILESIZE_LARGE, reader_buffer_->Size());
  //
  auto headers_up = GetHeaders();
  if (headers_up) {
    XSET_EASY_OPT(curl_, CURLOPT_HTTPHEADER, headers_up.get());
  }

  if (!chain_->Client().Proxy().empty()) {
    XSET_EASY_OPT(curl_, CURLOPT_PROXY, chain_->Client().Proxy().c_str());
  }

  auto code = curl_easy_perform(curl_);
  if (code != CURLE_OK) {
    LOG(ERROR) << strings::FormatString(
        "failed to curl_easy_perform, code: %d, message: %s", code,
        error_message_);
    return code;
  }
  long status = 0;
  code = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &status);
  if (code != CURLE_OK) {
    LOG(ERROR) << strings::FormatString(
        "failed to curl_easy_getinfo, code: %d, message: %s", code,
        curl_easy_strerror(code));
    return code;
  }
  chain_->Response().SetStatusCode(status);

  if (status >= 400) {
    LOG(ERROR) << strings::FormatString("http status code: %d", status);
    return status;
  }

  return chain_->Handler();
}

size_t CurlClient::writeCallback(char *ptr, size_t size, size_t nmemb,
                                 void *userdata) {
  LOG(INFO) << "=======writeCallback=========";
  if (ptr == nullptr || userdata == nullptr) {
    LOG(ERROR) << "ptr: " << ptr << ", userdata: " << userdata;
    return 0;
  }

  CurlClient *client = static_cast<CurlClient *>(userdata);
  if (client == nullptr) {
    LOG(ERROR) << "client is nil";
    return -1;
  }

  size_t len = size * nmemb;
  client->chain_->Response().AppendBody(std::string(ptr, len));

  LOG(INFO) << "write data len: " << len
            << ", resp body len: " << client->chain_->Response().Body().size();
  return len;
}

size_t CurlClient::readCallback(char *ptr, size_t size, size_t nmemb,
                                void *userdata) {
  CurlClient *client = static_cast<CurlClient *>(userdata);
  if (client == nullptr) {
    return 0;
  }

  size_t buffer_size = size * nmemb;
  if (buffer_size <= 0) {
    return 0;
  }

  std::string buffer;
  buffer.resize(buffer_size);

  int ret = client->reader_buffer_->Read(buffer);
  if (ret != 0) {
    LOG(ERROR) << "failed to read in read callback, ret: " << ret;
    return ret;
  }
  LOG(INFO) << "read data: " << buffer;

  return buffer_size;
}

std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)>
CurlClient::GetHeaders() const {
  auto list_up = std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)>(
      0, curl_slist_free_all);
  curl_slist *list = nullptr;

  chain_->Request().Headers().Traverse(
      [&](const std::string &k, const std::string &v) {
        curl_slist_append(list, (k + ":" + v).c_str());
      });

  list_up.reset(list);
  return list_up;
}

}  // namespace net
}  // namespace kingfisher
