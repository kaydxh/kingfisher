#ifndef KINGFISHER_PKG_MIDDLEWARE_BRPC_MIDDLEWARE_IN_OUTPUT_PRINTER_H_
#define KINGFISHER_PKG_MIDDLEWARE_BRPC_MIDDLEWARE_IN_OUTPUT_PRINTER_H_

#include "brpc/controller.h"
#include "brpc/server.h"

/*
class InOutputPrinterInterceptor : public brpc::NamingServiceFilter {
 public:
#if 0
  bool Process(brpc::Controller* cntl, const google::protobuf::Message* request,
               google::protobuf::Message* response,
               brpc::InterceptorContext* context) {
    LOG(INFO) << "request: " << request->ShortDebugString();
    if (!context->Next()) {
      return false;
    }

    LOG(INF) << "response: " << response->ShortDebugString();

    return true
  }

#endif
  bool Accept(const brpc::Controller* controller, int& error_code,
              std::string& error_txt) const {
    return true;
  }
};
*/

void InputPrinter(brpc::InputMessageBase* msg_base) {
  brpc::Controller* cntl = static_cast<brpc::Controller*>(msg_base);
  const google::protobuf::Message* request = cntl->request();
  if (request) {
    LOG(INFO) << "Request: " << request->ShortDebugString();
  } else {
    LOG(INFO) << "Request is nullptr";
  }
}

#endif
