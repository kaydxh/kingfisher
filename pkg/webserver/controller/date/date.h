#ifndef KINGFISHER_PKG_WEB_SERVER_CONTROLLER_DATE_DATE_H_
#define KINGFISHER_PKG_WEB_SERVER_CONTROLLER_DATE_DATE_H_

#include "time/timestamp.h"
#include "webserver/webserver.h"

namespace kingfisher {
namespace web {

// DateWebHandler: 日期服务 WebHandler
// 对标 golang 项目中 DateService，同时支持 HTTP
class DateWebHandler : public WebHandler {
 public:
  void SetRoutes(httplib::Server& http_server) override {
    // HTTP 路由：GET /api/date/now
    http_server.Get("/api/date/now",
                    [](const httplib::Request& req, httplib::Response& resp) {
                      auto now =
                          kingfisher::time::Timestamp::Now().ToFormattedString();

                      std::string request_id =
                          resp.get_header_value("X-Request-Id");

                      std::string json_resp =
                          R"({"request_id":")" + request_id +
                          R"(","date":")" + now + "\"}";

                      resp.status = 200;
                      resp.set_content(json_resp, "application/json");
                    });
  }

#ifdef ENABLE_GRPC
  // gRPC service 注册（如果用户定义了 gRPC proto service）
  void RegisterGRPCService(grpc::ServerBuilder& builder) override {
    // 用户可以在这里注册 gRPC service
    // 例如: builder.RegisterService(&date_grpc_service_);
  }
#endif
};

}  // namespace web
}  // namespace kingfisher

#endif

