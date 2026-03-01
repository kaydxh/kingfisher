#ifndef KINGFISHER_PKG_MIDDLEWARE_GRPC_MIDDLEWARE_REQUEST_ID_H_
#define KINGFISHER_PKG_MIDDLEWARE_GRPC_MIDDLEWARE_REQUEST_ID_H_

#include <string>

#include "google/protobuf/message.h"
#include "middleware/api/request_id.h"
#include "uuid/guid.h"

namespace kingfisher {
namespace middleware {
namespace grpc {

// gRPC RequestID 中间件：从请求中提取或生成 request_id
// 对标 golang 的 grpc-middleware/debug/request_id_server.interceptor.go
//
// 用法：在 gRPC service handler 中调用
//   std::string request_id = grpc::RequestID(req);
inline std::string RequestID(::google::protobuf::Message* request) {
  // 先尝试从 protobuf 消息中提取 request_id 字段
  std::string request_id =
      kingfisher::middleware::ExtractStringField(request, "request_id");

  if (request_id.empty()) {
    // 如果没有 request_id 字段，则生成新的
    request_id = kingfisher::uuid::Guid::GuidString();

    // 尝试将生成的 request_id 写回请求消息
    kingfisher::middleware::RequestID(request);
  }

  return request_id;
}

// TrySetResponseRequestID：尝试将 request_id 写入响应消息
// 对标 golang 中的 reflect_.TrySetId(resp, reflect_.FieldNameRequestId, id)
inline void TrySetResponseRequestID(::google::protobuf::Message* response,
                                    const std::string& request_id) {
  if (response == nullptr) return;

  const ::google::protobuf::Descriptor* descriptor =
      response->GetDescriptor();
  const ::google::protobuf::FieldDescriptor* field =
      descriptor->FindFieldByName("request_id");

  if (field &&
      field->type() == ::google::protobuf::FieldDescriptor::TYPE_STRING) {
    response->GetReflection()->SetString(response, field, request_id);
  }
}

}  // namespace grpc
}  // namespace middleware
}  // namespace kingfisher

#endif
