#include "http_headers.h"

#include <string>

namespace kingfisher {
namespace net {

HttpHeaders::HttpHeaders() {}
HttpHeaders::~HttpHeaders() {}

void HttpHeaders::SetHeader(const std::string& key, const std::string& value) {
  headers_.erase(key);
  headers_.insert(std::pair<std::string, std::string>(key, value));
}

void HttpHeaders::AppendHeader(const std::string& key,
                               const std::string& value) {
  headers_.insert(std::pair<std::string, std::string>(key, value));
}

std::string HttpHeaders::GetHeader(const std::string& key) {
  auto range = headers_.equal_range(key);
  auto it = range.first;
  if (it != range.second) {
    return it->second;
  }

  return "";
}

std::vector<std::string> HttpHeaders::GetHeaders(const std::string& key) {
  std::vector<std::string> headers;
  auto range = headers_.equal_range(key);
  for (auto it = range.first; it != range.second; ++it) {
    headers.push_back(it->second);
  }

  return headers;
}

void HttpHeaders::SetContentLength(int64_t sz) {
  SetHeader("Content-Length", std::to_string(sz));
}

void HttpHeaders::RemoveHeaders(const std::string& key) { headers_.erase(key); }

void HttpHeaders::RemoveHeader(const std::string& key,
                               const std::string& value) {
  auto range = headers_.equal_range(key);
  for (auto it = range.first; it != range.second;) {
    if (it->second == value) {
      it = headers_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace net
}  // namespace kingfisher
