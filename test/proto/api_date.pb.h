// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: test/proto/api_date.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_test_2fproto_2fapi_5fdate_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_test_2fproto_2fapi_5fdate_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3012000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3012002 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/descriptor.pb.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_test_2fproto_2fapi_5fdate_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_test_2fproto_2fapi_5fdate_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[2]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_test_2fproto_2fapi_5fdate_2eproto;
namespace sea {
namespace api {
namespace seadate {
class NowRequest;
class NowRequestDefaultTypeInternal;
extern NowRequestDefaultTypeInternal _NowRequest_default_instance_;
class NowResponse;
class NowResponseDefaultTypeInternal;
extern NowResponseDefaultTypeInternal _NowResponse_default_instance_;
}  // namespace seadate
}  // namespace api
}  // namespace sea
PROTOBUF_NAMESPACE_OPEN
template<> ::sea::api::seadate::NowRequest* Arena::CreateMaybeMessage<::sea::api::seadate::NowRequest>(Arena*);
template<> ::sea::api::seadate::NowResponse* Arena::CreateMaybeMessage<::sea::api::seadate::NowResponse>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace sea {
namespace api {
namespace seadate {

// ===================================================================

class NowRequest PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:sea.api.seadate.NowRequest) */ {
 public:
  inline NowRequest() : NowRequest(nullptr) {};
  virtual ~NowRequest();

  NowRequest(const NowRequest& from);
  NowRequest(NowRequest&& from) noexcept
    : NowRequest() {
    *this = ::std::move(from);
  }

  inline NowRequest& operator=(const NowRequest& from) {
    CopyFrom(from);
    return *this;
  }
  inline NowRequest& operator=(NowRequest&& from) noexcept {
    if (GetArena() == from.GetArena()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const NowRequest& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const NowRequest* internal_default_instance() {
    return reinterpret_cast<const NowRequest*>(
               &_NowRequest_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(NowRequest& a, NowRequest& b) {
    a.Swap(&b);
  }
  inline void Swap(NowRequest* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(NowRequest* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline NowRequest* New() const final {
    return CreateMaybeMessage<NowRequest>(nullptr);
  }

  NowRequest* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<NowRequest>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const NowRequest& from);
  void MergeFrom(const NowRequest& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(NowRequest* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "sea.api.seadate.NowRequest";
  }
  protected:
  explicit NowRequest(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_test_2fproto_2fapi_5fdate_2eproto);
    return ::descriptor_table_test_2fproto_2fapi_5fdate_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kRequestIdFieldNumber = 1,
  };
  // string request_id = 1[json_name = "RequestId"];
  void clear_request_id();
  const std::string& request_id() const;
  void set_request_id(const std::string& value);
  void set_request_id(std::string&& value);
  void set_request_id(const char* value);
  void set_request_id(const char* value, size_t size);
  std::string* mutable_request_id();
  std::string* release_request_id();
  void set_allocated_request_id(std::string* request_id);
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  std::string* unsafe_arena_release_request_id();
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  void unsafe_arena_set_allocated_request_id(
      std::string* request_id);
  private:
  const std::string& _internal_request_id() const;
  void _internal_set_request_id(const std::string& value);
  std::string* _internal_mutable_request_id();
  public:

  // @@protoc_insertion_point(class_scope:sea.api.seadate.NowRequest)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr request_id_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_test_2fproto_2fapi_5fdate_2eproto;
};
// -------------------------------------------------------------------

class NowResponse PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:sea.api.seadate.NowResponse) */ {
 public:
  inline NowResponse() : NowResponse(nullptr) {};
  virtual ~NowResponse();

  NowResponse(const NowResponse& from);
  NowResponse(NowResponse&& from) noexcept
    : NowResponse() {
    *this = ::std::move(from);
  }

  inline NowResponse& operator=(const NowResponse& from) {
    CopyFrom(from);
    return *this;
  }
  inline NowResponse& operator=(NowResponse&& from) noexcept {
    if (GetArena() == from.GetArena()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const NowResponse& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const NowResponse* internal_default_instance() {
    return reinterpret_cast<const NowResponse*>(
               &_NowResponse_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(NowResponse& a, NowResponse& b) {
    a.Swap(&b);
  }
  inline void Swap(NowResponse* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(NowResponse* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline NowResponse* New() const final {
    return CreateMaybeMessage<NowResponse>(nullptr);
  }

  NowResponse* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<NowResponse>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const NowResponse& from);
  void MergeFrom(const NowResponse& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(NowResponse* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "sea.api.seadate.NowResponse";
  }
  protected:
  explicit NowResponse(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_test_2fproto_2fapi_5fdate_2eproto);
    return ::descriptor_table_test_2fproto_2fapi_5fdate_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kRequestIdFieldNumber = 1,
    kDateFieldNumber = 2,
  };
  // string request_id = 1[json_name = "RequestId"];
  void clear_request_id();
  const std::string& request_id() const;
  void set_request_id(const std::string& value);
  void set_request_id(std::string&& value);
  void set_request_id(const char* value);
  void set_request_id(const char* value, size_t size);
  std::string* mutable_request_id();
  std::string* release_request_id();
  void set_allocated_request_id(std::string* request_id);
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  std::string* unsafe_arena_release_request_id();
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  void unsafe_arena_set_allocated_request_id(
      std::string* request_id);
  private:
  const std::string& _internal_request_id() const;
  void _internal_set_request_id(const std::string& value);
  std::string* _internal_mutable_request_id();
  public:

  // string date = 2[json_name = "Date"];
  void clear_date();
  const std::string& date() const;
  void set_date(const std::string& value);
  void set_date(std::string&& value);
  void set_date(const char* value);
  void set_date(const char* value, size_t size);
  std::string* mutable_date();
  std::string* release_date();
  void set_allocated_date(std::string* date);
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  std::string* unsafe_arena_release_date();
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  void unsafe_arena_set_allocated_date(
      std::string* date);
  private:
  const std::string& _internal_date() const;
  void _internal_set_date(const std::string& value);
  std::string* _internal_mutable_date();
  public:

  // @@protoc_insertion_point(class_scope:sea.api.seadate.NowResponse)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr request_id_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr date_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_test_2fproto_2fapi_5fdate_2eproto;
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
  request_id_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& NowRequest::request_id() const {
  // @@protoc_insertion_point(field_get:sea.api.seadate.NowRequest.request_id)
  return _internal_request_id();
}
inline void NowRequest::set_request_id(const std::string& value) {
  _internal_set_request_id(value);
  // @@protoc_insertion_point(field_set:sea.api.seadate.NowRequest.request_id)
}
inline std::string* NowRequest::mutable_request_id() {
  // @@protoc_insertion_point(field_mutable:sea.api.seadate.NowRequest.request_id)
  return _internal_mutable_request_id();
}
inline const std::string& NowRequest::_internal_request_id() const {
  return request_id_.Get();
}
inline void NowRequest::_internal_set_request_id(const std::string& value) {
  
  request_id_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void NowRequest::set_request_id(std::string&& value) {
  
  request_id_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:sea.api.seadate.NowRequest.request_id)
}
inline void NowRequest::set_request_id(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  request_id_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:sea.api.seadate.NowRequest.request_id)
}
inline void NowRequest::set_request_id(const char* value,
    size_t size) {
  
  request_id_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:sea.api.seadate.NowRequest.request_id)
}
inline std::string* NowRequest::_internal_mutable_request_id() {
  
  return request_id_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* NowRequest::release_request_id() {
  // @@protoc_insertion_point(field_release:sea.api.seadate.NowRequest.request_id)
  return request_id_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void NowRequest::set_allocated_request_id(std::string* request_id) {
  if (request_id != nullptr) {
    
  } else {
    
  }
  request_id_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), request_id,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:sea.api.seadate.NowRequest.request_id)
}
inline std::string* NowRequest::unsafe_arena_release_request_id() {
  // @@protoc_insertion_point(field_unsafe_arena_release:sea.api.seadate.NowRequest.request_id)
  GOOGLE_DCHECK(GetArena() != nullptr);
  
  return request_id_.UnsafeArenaRelease(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      GetArena());
}
inline void NowRequest::unsafe_arena_set_allocated_request_id(
    std::string* request_id) {
  GOOGLE_DCHECK(GetArena() != nullptr);
  if (request_id != nullptr) {
    
  } else {
    
  }
  request_id_.UnsafeArenaSetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      request_id, GetArena());
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:sea.api.seadate.NowRequest.request_id)
}

// -------------------------------------------------------------------

// NowResponse

// string request_id = 1[json_name = "RequestId"];
inline void NowResponse::clear_request_id() {
  request_id_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& NowResponse::request_id() const {
  // @@protoc_insertion_point(field_get:sea.api.seadate.NowResponse.request_id)
  return _internal_request_id();
}
inline void NowResponse::set_request_id(const std::string& value) {
  _internal_set_request_id(value);
  // @@protoc_insertion_point(field_set:sea.api.seadate.NowResponse.request_id)
}
inline std::string* NowResponse::mutable_request_id() {
  // @@protoc_insertion_point(field_mutable:sea.api.seadate.NowResponse.request_id)
  return _internal_mutable_request_id();
}
inline const std::string& NowResponse::_internal_request_id() const {
  return request_id_.Get();
}
inline void NowResponse::_internal_set_request_id(const std::string& value) {
  
  request_id_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void NowResponse::set_request_id(std::string&& value) {
  
  request_id_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:sea.api.seadate.NowResponse.request_id)
}
inline void NowResponse::set_request_id(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  request_id_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:sea.api.seadate.NowResponse.request_id)
}
inline void NowResponse::set_request_id(const char* value,
    size_t size) {
  
  request_id_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:sea.api.seadate.NowResponse.request_id)
}
inline std::string* NowResponse::_internal_mutable_request_id() {
  
  return request_id_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* NowResponse::release_request_id() {
  // @@protoc_insertion_point(field_release:sea.api.seadate.NowResponse.request_id)
  return request_id_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void NowResponse::set_allocated_request_id(std::string* request_id) {
  if (request_id != nullptr) {
    
  } else {
    
  }
  request_id_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), request_id,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:sea.api.seadate.NowResponse.request_id)
}
inline std::string* NowResponse::unsafe_arena_release_request_id() {
  // @@protoc_insertion_point(field_unsafe_arena_release:sea.api.seadate.NowResponse.request_id)
  GOOGLE_DCHECK(GetArena() != nullptr);
  
  return request_id_.UnsafeArenaRelease(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      GetArena());
}
inline void NowResponse::unsafe_arena_set_allocated_request_id(
    std::string* request_id) {
  GOOGLE_DCHECK(GetArena() != nullptr);
  if (request_id != nullptr) {
    
  } else {
    
  }
  request_id_.UnsafeArenaSetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      request_id, GetArena());
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:sea.api.seadate.NowResponse.request_id)
}

// string date = 2[json_name = "Date"];
inline void NowResponse::clear_date() {
  date_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& NowResponse::date() const {
  // @@protoc_insertion_point(field_get:sea.api.seadate.NowResponse.date)
  return _internal_date();
}
inline void NowResponse::set_date(const std::string& value) {
  _internal_set_date(value);
  // @@protoc_insertion_point(field_set:sea.api.seadate.NowResponse.date)
}
inline std::string* NowResponse::mutable_date() {
  // @@protoc_insertion_point(field_mutable:sea.api.seadate.NowResponse.date)
  return _internal_mutable_date();
}
inline const std::string& NowResponse::_internal_date() const {
  return date_.Get();
}
inline void NowResponse::_internal_set_date(const std::string& value) {
  
  date_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void NowResponse::set_date(std::string&& value) {
  
  date_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:sea.api.seadate.NowResponse.date)
}
inline void NowResponse::set_date(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  date_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:sea.api.seadate.NowResponse.date)
}
inline void NowResponse::set_date(const char* value,
    size_t size) {
  
  date_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:sea.api.seadate.NowResponse.date)
}
inline std::string* NowResponse::_internal_mutable_date() {
  
  return date_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* NowResponse::release_date() {
  // @@protoc_insertion_point(field_release:sea.api.seadate.NowResponse.date)
  return date_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void NowResponse::set_allocated_date(std::string* date) {
  if (date != nullptr) {
    
  } else {
    
  }
  date_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), date,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:sea.api.seadate.NowResponse.date)
}
inline std::string* NowResponse::unsafe_arena_release_date() {
  // @@protoc_insertion_point(field_unsafe_arena_release:sea.api.seadate.NowResponse.date)
  GOOGLE_DCHECK(GetArena() != nullptr);
  
  return date_.UnsafeArenaRelease(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      GetArena());
}
inline void NowResponse::unsafe_arena_set_allocated_date(
    std::string* date) {
  GOOGLE_DCHECK(GetArena() != nullptr);
  if (date != nullptr) {
    
  } else {
    
  }
  date_.UnsafeArenaSetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      date, GetArena());
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:sea.api.seadate.NowResponse.date)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace seadate
}  // namespace api
}  // namespace sea

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_test_2fproto_2fapi_5fdate_2eproto
