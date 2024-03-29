#include "http_client.h"

#include <memory>

#include "curl_client.h"
#include "log/config.h"
#include "net/http/chain_interceptor.h"
#include "net/http/types.h"

namespace kingfisher {
namespace net {

HttpClient::HttpClient() {}

HttpClient::~HttpClient() {}

int HttpClient::Init() { return 0; }

int HttpClient::Get(HttpRequest& req, HttpResponse& resp) {
  req.SetMethod(HTTP_METHOD_GET);
  return Do(req, resp);
}

int HttpClient::Post(HttpRequest& req, HttpResponse& resp) {
  req.SetMethod(HTTP_METHOD_POST);
  return Do(req, resp);
}

int HttpClient::Put(HttpRequest& req, HttpResponse& resp) {
  req.SetMethod(HTTP_METHOD_PUT);
  return Do(req, resp);
}

int HttpClient::Do(HttpRequest& req, HttpResponse& resp) {
  auto interceptors = interceptors_;
  auto curl_client = std::make_shared<CurlClient>();
  if (curl_client->Init() != 0) {
    LOG(ERROR) << "failed to init curl client";
    return -1;
  }
  interceptors.push_back(curl_client);
  HttpChainInterceptor chain(*this, req, resp, interceptors.begin(),
                             interceptors.end());

  return chain.Handler();
}

void HttpClient::SetProxy(const std::string& proxy) { proxy_ = proxy; }

void HttpClient::AddInterceptor(
    const std::shared_ptr<HttpInterceptor> interceptor) {
  interceptors_.push_back(interceptor);
}

}  // namespace net
}  // namespace kingfisher
