#ifndef KINGFISHER_PKG_WEB_SERVER_CONTROLLER_DATE_DATE_H_
#define KINGFISHER_PKG_WEB_SERVER_CONTROLLER_DATE_DATE_H_

#include "api.pb.h"
#include "brpc/server.h"
#include "middleware/api/api_guard.hpp"
#include "time/timestamp.h"

// curl http://127.0.0.1:10000/SeaDateService/Now
class DateServiceImpl : public date::SeaDateService {
 public:
  void Now(::google::protobuf::RpcController* cntl_base,
           const date::NowRequest* req, date::NowResponse* resp,
           ::google::protobuf::Closure* done) override {
    // This object helps you to call done->Run() in RAII style. If you need
    // to process the request asynchronously, pass done_guard.release().
    brpc::ClosureGuard done_guard(done);
    API_GUARD;

    brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
    cntl->http_response().set_content_type("text/plain");

    butil::IOBufBuilder os;
    auto now = kingfisher::time::Timestamp::Now().ToFormattedString();
    os << now;
    os.move_to(cntl->response_attachment());

    resp->set_request_id(req->request_id());
    resp->set_date(now);
  }
};

#endif