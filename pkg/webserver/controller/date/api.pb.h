// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: pkg/webserver/controller/date/api.proto

#ifndef PROTOBUF_INCLUDED_pkg_2fwebserver_2fcontroller_2fdate_2fapi_2eproto
#define PROTOBUF_INCLUDED_pkg_2fwebserver_2fcontroller_2fdate_2fapi_2eproto

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3006001
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/service.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/descriptor.pb.h>
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_pkg_2fwebserver_2fcontroller_2fdate_2fapi_2eproto 

namespace protobuf_pkg_2fwebserver_2fcontroller_2fdate_2fapi_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[4];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
}  // namespace protobuf_pkg_2fwebserver_2fcontroller_2fdate_2fapi_2eproto
namespace date {
class NowErrorRequest;
class NowErrorRequestDefaultTypeInternal;
extern NowErrorRequestDefaultTypeInternal _NowErrorRequest_default_instance_;
class NowErrorResponse;
class NowErrorResponseDefaultTypeInternal;
extern NowErrorResponseDefaultTypeInternal _NowErrorResponse_default_instance_;
class NowRequest;
class NowRequestDefaultTypeInternal;
extern NowRequestDefaultTypeInternal _NowRequest_default_instance_;
class NowResponse;
class NowResponseDefaultTypeInternal;
extern NowResponseDefaultTypeInternal _NowResponse_default_instance_;
}  // namespace date
namespace google {
namespace protobuf {
template<> ::date::NowErrorRequest* Arena::CreateMaybeMessage<::date::NowErrorRequest>(Arena*);
template<> ::date::NowErrorResponse* Arena::CreateMaybeMessage<::date::NowErrorResponse>(Arena*);
template<> ::date::NowRequest* Arena::CreateMaybeMessage<::date::NowRequest>(Arena*);
template<> ::date::NowResponse* Arena::CreateMaybeMessage<::date::NowResponse>(Arena*);
}  // namespace protobuf
}  // namespace google
namespace date {

// ===================================================================

class NowRequest : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:date.NowRequest) */ {
 public:
  NowRequest();
  virtual ~NowRequest();

  NowRequest(const NowRequest& from);

  inline NowRequest& operator=(const NowRequest& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  NowRequest(NowRequest&& from) noexcept
    : NowRequest() {
    *this = ::std::move(from);
  }

  inline NowRequest& operator=(NowRequest&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const NowRequest& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const NowRequest* internal_default_instance() {
    return reinterpret_cast<const NowRequest*>(
               &_NowRequest_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(NowRequest* other);
  friend void swap(NowRequest& a, NowRequest& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline NowRequest* New() const final {
    return CreateMaybeMessage<NowRequest>(NULL);
  }

  NowRequest* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<NowRequest>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const NowRequest& from);
  void MergeFrom(const NowRequest& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(NowRequest* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string request_id = 1[json_name = "RequestId"];
  void clear_request_id();
  static const int kRequestIdFieldNumber = 1;
  const ::std::string& request_id() const;
  void set_request_id(const ::std::string& value);
  #if LANG_CXX11
  void set_request_id(::std::string&& value);
  #endif
  void set_request_id(const char* value);
  void set_request_id(const char* value, size_t size);
  ::std::string* mutable_request_id();
  ::std::string* release_request_id();
  void set_allocated_request_id(::std::string* request_id);

  // @@protoc_insertion_point(class_scope:date.NowRequest)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr request_id_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_pkg_2fwebserver_2fcontroller_2fdate_2fapi_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class NowResponse : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:date.NowResponse) */ {
 public:
  NowResponse();
  virtual ~NowResponse();

  NowResponse(const NowResponse& from);

  inline NowResponse& operator=(const NowResponse& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  NowResponse(NowResponse&& from) noexcept
    : NowResponse() {
    *this = ::std::move(from);
  }

  inline NowResponse& operator=(NowResponse&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const NowResponse& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const NowResponse* internal_default_instance() {
    return reinterpret_cast<const NowResponse*>(
               &_NowResponse_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  void Swap(NowResponse* other);
  friend void swap(NowResponse& a, NowResponse& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline NowResponse* New() const final {
    return CreateMaybeMessage<NowResponse>(NULL);
  }

  NowResponse* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<NowResponse>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const NowResponse& from);
  void MergeFrom(const NowResponse& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(NowResponse* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string request_id = 1[json_name = "RequestId"];
  void clear_request_id();
  static const int kRequestIdFieldNumber = 1;
  const ::std::string& request_id() const;
  void set_request_id(const ::std::string& value);
  #if LANG_CXX11
  void set_request_id(::std::string&& value);
  #endif
  void set_request_id(const char* value);
  void set_request_id(const char* value, size_t size);
  ::std::string* mutable_request_id();
  ::std::string* release_request_id();
  void set_allocated_request_id(::std::string* request_id);

  // string date = 2[json_name = "Date"];
  void clear_date();
  static const int kDateFieldNumber = 2;
  const ::std::string& date() const;
  void set_date(const ::std::string& value);
  #if LANG_CXX11
  void set_date(::std::string&& value);
  #endif
  void set_date(const char* value);
  void set_date(const char* value, size_t size);
  ::std::string* mutable_date();
  ::std::string* release_date();
  void set_allocated_date(::std::string* date);

  // @@protoc_insertion_point(class_scope:date.NowResponse)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr request_id_;
  ::google::protobuf::internal::ArenaStringPtr date_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_pkg_2fwebserver_2fcontroller_2fdate_2fapi_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class NowErrorRequest : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:date.NowErrorRequest) */ {
 public:
  NowErrorRequest();
  virtual ~NowErrorRequest();

  NowErrorRequest(const NowErrorRequest& from);

  inline NowErrorRequest& operator=(const NowErrorRequest& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  NowErrorRequest(NowErrorRequest&& from) noexcept
    : NowErrorRequest() {
    *this = ::std::move(from);
  }

  inline NowErrorRequest& operator=(NowErrorRequest&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const NowErrorRequest& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const NowErrorRequest* internal_default_instance() {
    return reinterpret_cast<const NowErrorRequest*>(
               &_NowErrorRequest_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    2;

  void Swap(NowErrorRequest* other);
  friend void swap(NowErrorRequest& a, NowErrorRequest& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline NowErrorRequest* New() const final {
    return CreateMaybeMessage<NowErrorRequest>(NULL);
  }

  NowErrorRequest* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<NowErrorRequest>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const NowErrorRequest& from);
  void MergeFrom(const NowErrorRequest& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(NowErrorRequest* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string request_id = 1[json_name = "RequestId"];
  void clear_request_id();
  static const int kRequestIdFieldNumber = 1;
  const ::std::string& request_id() const;
  void set_request_id(const ::std::string& value);
  #if LANG_CXX11
  void set_request_id(::std::string&& value);
  #endif
  void set_request_id(const char* value);
  void set_request_id(const char* value, size_t size);
  ::std::string* mutable_request_id();
  ::std::string* release_request_id();
  void set_allocated_request_id(::std::string* request_id);

  // @@protoc_insertion_point(class_scope:date.NowErrorRequest)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr request_id_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_pkg_2fwebserver_2fcontroller_2fdate_2fapi_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class NowErrorResponse : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:date.NowErrorResponse) */ {
 public:
  NowErrorResponse();
  virtual ~NowErrorResponse();

  NowErrorResponse(const NowErrorResponse& from);

  inline NowErrorResponse& operator=(const NowErrorResponse& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  NowErrorResponse(NowErrorResponse&& from) noexcept
    : NowErrorResponse() {
    *this = ::std::move(from);
  }

  inline NowErrorResponse& operator=(NowErrorResponse&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const NowErrorResponse& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const NowErrorResponse* internal_default_instance() {
    return reinterpret_cast<const NowErrorResponse*>(
               &_NowErrorResponse_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    3;

  void Swap(NowErrorResponse* other);
  friend void swap(NowErrorResponse& a, NowErrorResponse& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline NowErrorResponse* New() const final {
    return CreateMaybeMessage<NowErrorResponse>(NULL);
  }

  NowErrorResponse* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<NowErrorResponse>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const NowErrorResponse& from);
  void MergeFrom(const NowErrorResponse& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(NowErrorResponse* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string request_id = 1[json_name = "RequestId"];
  void clear_request_id();
  static const int kRequestIdFieldNumber = 1;
  const ::std::string& request_id() const;
  void set_request_id(const ::std::string& value);
  #if LANG_CXX11
  void set_request_id(::std::string&& value);
  #endif
  void set_request_id(const char* value);
  void set_request_id(const char* value, size_t size);
  ::std::string* mutable_request_id();
  ::std::string* release_request_id();
  void set_allocated_request_id(::std::string* request_id);

  // string date = 2[json_name = "Date"];
  void clear_date();
  static const int kDateFieldNumber = 2;
  const ::std::string& date() const;
  void set_date(const ::std::string& value);
  #if LANG_CXX11
  void set_date(::std::string&& value);
  #endif
  void set_date(const char* value);
  void set_date(const char* value, size_t size);
  ::std::string* mutable_date();
  ::std::string* release_date();
  void set_allocated_date(::std::string* date);

  // @@protoc_insertion_point(class_scope:date.NowErrorResponse)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr request_id_;
  ::google::protobuf::internal::ArenaStringPtr date_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_pkg_2fwebserver_2fcontroller_2fdate_2fapi_2eproto::TableStruct;
};
// ===================================================================

class SeaDateService_Stub;

class SeaDateService : public ::google::protobuf::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline SeaDateService() {};
 public:
  virtual ~SeaDateService();

  typedef SeaDateService_Stub Stub;

  static const ::google::protobuf::ServiceDescriptor* descriptor();

  virtual void Now(::google::protobuf::RpcController* controller,
                       const ::date::NowRequest* request,
                       ::date::NowResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void NowError(::google::protobuf::RpcController* controller,
                       const ::date::NowErrorRequest* request,
                       ::date::NowErrorResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::google::protobuf::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                  ::google::protobuf::RpcController* controller,
                  const ::google::protobuf::Message* request,
                  ::google::protobuf::Message* response,
                  ::google::protobuf::Closure* done);
  const ::google::protobuf::Message& GetRequestPrototype(
    const ::google::protobuf::MethodDescriptor* method) const;
  const ::google::protobuf::Message& GetResponsePrototype(
    const ::google::protobuf::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(SeaDateService);
};

class SeaDateService_Stub : public SeaDateService {
 public:
  SeaDateService_Stub(::google::protobuf::RpcChannel* channel);
  SeaDateService_Stub(::google::protobuf::RpcChannel* channel,
                   ::google::protobuf::Service::ChannelOwnership ownership);
  ~SeaDateService_Stub();

  inline ::google::protobuf::RpcChannel* channel() { return channel_; }

  // implements SeaDateService ------------------------------------------

  void Now(::google::protobuf::RpcController* controller,
                       const ::date::NowRequest* request,
                       ::date::NowResponse* response,
                       ::google::protobuf::Closure* done);
  void NowError(::google::protobuf::RpcController* controller,
                       const ::date::NowErrorRequest* request,
                       ::date::NowErrorResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::google::protobuf::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(SeaDateService_Stub);
};


// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// NowRequest

// string request_id = 1[json_name = "RequestId"];
inline void NowRequest::clear_request_id() {
  request_id_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& NowRequest::request_id() const {
  // @@protoc_insertion_point(field_get:date.NowRequest.request_id)
  return request_id_.GetNoArena();
}
inline void NowRequest::set_request_id(const ::std::string& value) {
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:date.NowRequest.request_id)
}
#if LANG_CXX11
inline void NowRequest::set_request_id(::std::string&& value) {
  
  request_id_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:date.NowRequest.request_id)
}
#endif
inline void NowRequest::set_request_id(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:date.NowRequest.request_id)
}
inline void NowRequest::set_request_id(const char* value, size_t size) {
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:date.NowRequest.request_id)
}
inline ::std::string* NowRequest::mutable_request_id() {
  
  // @@protoc_insertion_point(field_mutable:date.NowRequest.request_id)
  return request_id_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* NowRequest::release_request_id() {
  // @@protoc_insertion_point(field_release:date.NowRequest.request_id)
  
  return request_id_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void NowRequest::set_allocated_request_id(::std::string* request_id) {
  if (request_id != NULL) {
    
  } else {
    
  }
  request_id_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), request_id);
  // @@protoc_insertion_point(field_set_allocated:date.NowRequest.request_id)
}

// -------------------------------------------------------------------

// NowResponse

// string request_id = 1[json_name = "RequestId"];
inline void NowResponse::clear_request_id() {
  request_id_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& NowResponse::request_id() const {
  // @@protoc_insertion_point(field_get:date.NowResponse.request_id)
  return request_id_.GetNoArena();
}
inline void NowResponse::set_request_id(const ::std::string& value) {
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:date.NowResponse.request_id)
}
#if LANG_CXX11
inline void NowResponse::set_request_id(::std::string&& value) {
  
  request_id_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:date.NowResponse.request_id)
}
#endif
inline void NowResponse::set_request_id(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:date.NowResponse.request_id)
}
inline void NowResponse::set_request_id(const char* value, size_t size) {
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:date.NowResponse.request_id)
}
inline ::std::string* NowResponse::mutable_request_id() {
  
  // @@protoc_insertion_point(field_mutable:date.NowResponse.request_id)
  return request_id_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* NowResponse::release_request_id() {
  // @@protoc_insertion_point(field_release:date.NowResponse.request_id)
  
  return request_id_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void NowResponse::set_allocated_request_id(::std::string* request_id) {
  if (request_id != NULL) {
    
  } else {
    
  }
  request_id_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), request_id);
  // @@protoc_insertion_point(field_set_allocated:date.NowResponse.request_id)
}

// string date = 2[json_name = "Date"];
inline void NowResponse::clear_date() {
  date_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& NowResponse::date() const {
  // @@protoc_insertion_point(field_get:date.NowResponse.date)
  return date_.GetNoArena();
}
inline void NowResponse::set_date(const ::std::string& value) {
  
  date_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:date.NowResponse.date)
}
#if LANG_CXX11
inline void NowResponse::set_date(::std::string&& value) {
  
  date_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:date.NowResponse.date)
}
#endif
inline void NowResponse::set_date(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  date_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:date.NowResponse.date)
}
inline void NowResponse::set_date(const char* value, size_t size) {
  
  date_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:date.NowResponse.date)
}
inline ::std::string* NowResponse::mutable_date() {
  
  // @@protoc_insertion_point(field_mutable:date.NowResponse.date)
  return date_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* NowResponse::release_date() {
  // @@protoc_insertion_point(field_release:date.NowResponse.date)
  
  return date_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void NowResponse::set_allocated_date(::std::string* date) {
  if (date != NULL) {
    
  } else {
    
  }
  date_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), date);
  // @@protoc_insertion_point(field_set_allocated:date.NowResponse.date)
}

// -------------------------------------------------------------------

// NowErrorRequest

// string request_id = 1[json_name = "RequestId"];
inline void NowErrorRequest::clear_request_id() {
  request_id_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& NowErrorRequest::request_id() const {
  // @@protoc_insertion_point(field_get:date.NowErrorRequest.request_id)
  return request_id_.GetNoArena();
}
inline void NowErrorRequest::set_request_id(const ::std::string& value) {
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:date.NowErrorRequest.request_id)
}
#if LANG_CXX11
inline void NowErrorRequest::set_request_id(::std::string&& value) {
  
  request_id_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:date.NowErrorRequest.request_id)
}
#endif
inline void NowErrorRequest::set_request_id(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:date.NowErrorRequest.request_id)
}
inline void NowErrorRequest::set_request_id(const char* value, size_t size) {
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:date.NowErrorRequest.request_id)
}
inline ::std::string* NowErrorRequest::mutable_request_id() {
  
  // @@protoc_insertion_point(field_mutable:date.NowErrorRequest.request_id)
  return request_id_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* NowErrorRequest::release_request_id() {
  // @@protoc_insertion_point(field_release:date.NowErrorRequest.request_id)
  
  return request_id_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void NowErrorRequest::set_allocated_request_id(::std::string* request_id) {
  if (request_id != NULL) {
    
  } else {
    
  }
  request_id_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), request_id);
  // @@protoc_insertion_point(field_set_allocated:date.NowErrorRequest.request_id)
}

// -------------------------------------------------------------------

// NowErrorResponse

// string request_id = 1[json_name = "RequestId"];
inline void NowErrorResponse::clear_request_id() {
  request_id_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& NowErrorResponse::request_id() const {
  // @@protoc_insertion_point(field_get:date.NowErrorResponse.request_id)
  return request_id_.GetNoArena();
}
inline void NowErrorResponse::set_request_id(const ::std::string& value) {
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:date.NowErrorResponse.request_id)
}
#if LANG_CXX11
inline void NowErrorResponse::set_request_id(::std::string&& value) {
  
  request_id_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:date.NowErrorResponse.request_id)
}
#endif
inline void NowErrorResponse::set_request_id(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:date.NowErrorResponse.request_id)
}
inline void NowErrorResponse::set_request_id(const char* value, size_t size) {
  
  request_id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:date.NowErrorResponse.request_id)
}
inline ::std::string* NowErrorResponse::mutable_request_id() {
  
  // @@protoc_insertion_point(field_mutable:date.NowErrorResponse.request_id)
  return request_id_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* NowErrorResponse::release_request_id() {
  // @@protoc_insertion_point(field_release:date.NowErrorResponse.request_id)
  
  return request_id_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void NowErrorResponse::set_allocated_request_id(::std::string* request_id) {
  if (request_id != NULL) {
    
  } else {
    
  }
  request_id_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), request_id);
  // @@protoc_insertion_point(field_set_allocated:date.NowErrorResponse.request_id)
}

// string date = 2[json_name = "Date"];
inline void NowErrorResponse::clear_date() {
  date_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& NowErrorResponse::date() const {
  // @@protoc_insertion_point(field_get:date.NowErrorResponse.date)
  return date_.GetNoArena();
}
inline void NowErrorResponse::set_date(const ::std::string& value) {
  
  date_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:date.NowErrorResponse.date)
}
#if LANG_CXX11
inline void NowErrorResponse::set_date(::std::string&& value) {
  
  date_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:date.NowErrorResponse.date)
}
#endif
inline void NowErrorResponse::set_date(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  date_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:date.NowErrorResponse.date)
}
inline void NowErrorResponse::set_date(const char* value, size_t size) {
  
  date_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:date.NowErrorResponse.date)
}
inline ::std::string* NowErrorResponse::mutable_date() {
  
  // @@protoc_insertion_point(field_mutable:date.NowErrorResponse.date)
  return date_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* NowErrorResponse::release_date() {
  // @@protoc_insertion_point(field_release:date.NowErrorResponse.date)
  
  return date_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void NowErrorResponse::set_allocated_date(::std::string* date) {
  if (date != NULL) {
    
  } else {
    
  }
  date_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), date);
  // @@protoc_insertion_point(field_set_allocated:date.NowErrorResponse.date)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace date

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_pkg_2fwebserver_2fcontroller_2fdate_2fapi_2eproto