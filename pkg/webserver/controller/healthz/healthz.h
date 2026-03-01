#ifndef KINGFISHER_PKG_WEB_SERVER_CONTROLLER_HEALTHZ_HEALTHZ_H_
#define KINGFISHER_PKG_WEB_SERVER_CONTROLLER_HEALTHZ_HEALTHZ_H_

#include "webserver/webserver.h"

namespace kingfisher {
namespace web {

// HealthzWebHandler: 健康检查 WebHandler
// 注意：核心的 /healthz, /livez, /readyz 路由已经由 GenericWebServer 内置注册
// 这个 Handler 可以用于注册额外的自定义健康检查路由
class HealthzWebHandler : public WebHandler {
 public:
  void SetRoutes(httplib::Server& http_server) override {
    // /healthz/ping - 简单的 ping 检查
    http_server.Get("/healthz/ping",
                    [](const httplib::Request& req, httplib::Response& resp) {
                      resp.status = 200;
                      resp.set_content("pong", "text/plain");
                    });
  }
};

}  // namespace web
}  // namespace kingfisher

#endif
