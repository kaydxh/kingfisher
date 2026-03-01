#ifndef KINGFISHER_PKG_WEB_SERVER_BASE_H_
#define KINGFISHER_PKG_WEB_SERVER_BASE_H_

#include <string>

namespace kingfisher {
namespace web {

// WebServerBase: 抽象基类，用于自定义 WebServer 行为
// 对标 golang 项目中的 preparedGenericWebServer
class WebServerBase {
 public:
  WebServerBase() {}
  virtual ~WebServerBase() {}

  virtual std::string ServerName() = 0;

  // 服务运行前的准备
  virtual void PreRun() {}

  // 服务停止后的清理
  virtual void PostRun() {}
};

}  // namespace web
}  // namespace kingfisher

#endif
