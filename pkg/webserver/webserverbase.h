#ifndef KINGFISHER_PKG_WEB_SERVER_BASE_H_
#define KINGFISHER_PKG_WEB_SERVER_BASE_H_

#include <string>

namespace kingfisher {
namespace web {

class WebServerBase {
 public:
  WebServerBase(){};
  virtual ~WebServerBase(){};
  virtual std::string ServerName() = 0;

  virtual void Run();
  virtual void BeforrRun(void *args);
  virtual void AfterRun(void *args);
};

}  // namespace web
}  // namespace kingfisher

#endif
