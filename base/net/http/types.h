#ifndef KINGFISHER_BASE_NET_HTTP_TYPES_H_
#define KINGFISHER_BASE_NET_HTTP_TYPES_H_

#include <memory>
#include <vector>

namespace kingfisher {
namespace net {

class HttpInterceptor;
using HttpInterceptors = std::vector<std::shared_ptr<HttpInterceptor>>;

const std::string HTTP_METHOD_GET = "GET";
const std::string HTTP_METHOD_POST = "POST";
const std::string HTTP_METHOD_PUT = "PUT";
const std::string HTTP_METHOD_OPTIONS = "OPTIONS";
const std::string HTTP_METHOD_HEAD = "HEAD";
const std::string HTTP_METHOD_PATCH = "PATCH";
const std::string HTTP_METHOD_DELETE = "DELETE";

}  // namespace net
}  // namespace kingfisher

#endif
