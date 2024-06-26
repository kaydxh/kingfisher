// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: brpc/rpc_dump.proto

#ifndef PROTOBUF_INCLUDED_brpc_2frpc_5fdump_2eproto
#define PROTOBUF_INCLUDED_brpc_2frpc_5fdump_2eproto

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
#include <google/protobuf/unknown_field_set.h>
#include "brpc/options.pb.h"
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_brpc_2frpc_5fdump_2eproto 

namespace protobuf_brpc_2frpc_5fdump_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[1];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
}  // namespace protobuf_brpc_2frpc_5fdump_2eproto
namespace brpc {
class RpcDumpMeta;
class RpcDumpMetaDefaultTypeInternal;
extern RpcDumpMetaDefaultTypeInternal _RpcDumpMeta_default_instance_;
}  // namespace brpc
namespace google {
namespace protobuf {
template<> ::brpc::RpcDumpMeta* Arena::CreateMaybeMessage<::brpc::RpcDumpMeta>(Arena*);
}  // namespace protobuf
}  // namespace google
namespace brpc {

// ===================================================================

class RpcDumpMeta : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:brpc.RpcDumpMeta) */ {
 public:
  RpcDumpMeta();
  virtual ~RpcDumpMeta();

  RpcDumpMeta(const RpcDumpMeta& from);

  inline RpcDumpMeta& operator=(const RpcDumpMeta& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  RpcDumpMeta(RpcDumpMeta&& from) noexcept
    : RpcDumpMeta() {
    *this = ::std::move(from);
  }

  inline RpcDumpMeta& operator=(RpcDumpMeta&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RpcDumpMeta& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const RpcDumpMeta* internal_default_instance() {
    return reinterpret_cast<const RpcDumpMeta*>(
               &_RpcDumpMeta_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(RpcDumpMeta* other);
  friend void swap(RpcDumpMeta& a, RpcDumpMeta& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline RpcDumpMeta* New() const final {
    return CreateMaybeMessage<RpcDumpMeta>(NULL);
  }

  RpcDumpMeta* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<RpcDumpMeta>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const RpcDumpMeta& from);
  void MergeFrom(const RpcDumpMeta& from);
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
  void InternalSwap(RpcDumpMeta* other);
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

  // optional string service_name = 1;
  bool has_service_name() const;
  void clear_service_name();
  static const int kServiceNameFieldNumber = 1;
  const ::std::string& service_name() const;
  void set_service_name(const ::std::string& value);
  #if LANG_CXX11
  void set_service_name(::std::string&& value);
  #endif
  void set_service_name(const char* value);
  void set_service_name(const char* value, size_t size);
  ::std::string* mutable_service_name();
  ::std::string* release_service_name();
  void set_allocated_service_name(::std::string* service_name);

  // optional string method_name = 2;
  bool has_method_name() const;
  void clear_method_name();
  static const int kMethodNameFieldNumber = 2;
  const ::std::string& method_name() const;
  void set_method_name(const ::std::string& value);
  #if LANG_CXX11
  void set_method_name(::std::string&& value);
  #endif
  void set_method_name(const char* value);
  void set_method_name(const char* value, size_t size);
  ::std::string* mutable_method_name();
  ::std::string* release_method_name();
  void set_allocated_method_name(::std::string* method_name);

  // optional bytes authentication_data = 7;
  bool has_authentication_data() const;
  void clear_authentication_data();
  static const int kAuthenticationDataFieldNumber = 7;
  const ::std::string& authentication_data() const;
  void set_authentication_data(const ::std::string& value);
  #if LANG_CXX11
  void set_authentication_data(::std::string&& value);
  #endif
  void set_authentication_data(const char* value);
  void set_authentication_data(const void* value, size_t size);
  ::std::string* mutable_authentication_data();
  ::std::string* release_authentication_data();
  void set_allocated_authentication_data(::std::string* authentication_data);

  // optional bytes user_data = 8;
  bool has_user_data() const;
  void clear_user_data();
  static const int kUserDataFieldNumber = 8;
  const ::std::string& user_data() const;
  void set_user_data(const ::std::string& value);
  #if LANG_CXX11
  void set_user_data(::std::string&& value);
  #endif
  void set_user_data(const char* value);
  void set_user_data(const void* value, size_t size);
  ::std::string* mutable_user_data();
  ::std::string* release_user_data();
  void set_allocated_user_data(::std::string* user_data);

  // optional bytes nshead = 9;
  bool has_nshead() const;
  void clear_nshead();
  static const int kNsheadFieldNumber = 9;
  const ::std::string& nshead() const;
  void set_nshead(const ::std::string& value);
  #if LANG_CXX11
  void set_nshead(::std::string&& value);
  #endif
  void set_nshead(const char* value);
  void set_nshead(const void* value, size_t size);
  ::std::string* mutable_nshead();
  ::std::string* release_nshead();
  void set_allocated_nshead(::std::string* nshead);

  // optional int32 method_index = 3;
  bool has_method_index() const;
  void clear_method_index();
  static const int kMethodIndexFieldNumber = 3;
  ::google::protobuf::int32 method_index() const;
  void set_method_index(::google::protobuf::int32 value);

  // optional .brpc.CompressType compress_type = 4;
  bool has_compress_type() const;
  void clear_compress_type();
  static const int kCompressTypeFieldNumber = 4;
  ::brpc::CompressType compress_type() const;
  void set_compress_type(::brpc::CompressType value);

  // optional .brpc.ProtocolType protocol_type = 5;
  bool has_protocol_type() const;
  void clear_protocol_type();
  static const int kProtocolTypeFieldNumber = 5;
  ::brpc::ProtocolType protocol_type() const;
  void set_protocol_type(::brpc::ProtocolType value);

  // optional int32 attachment_size = 6;
  bool has_attachment_size() const;
  void clear_attachment_size();
  static const int kAttachmentSizeFieldNumber = 6;
  ::google::protobuf::int32 attachment_size() const;
  void set_attachment_size(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:brpc.RpcDumpMeta)
 private:
  void set_has_service_name();
  void clear_has_service_name();
  void set_has_method_name();
  void clear_has_method_name();
  void set_has_method_index();
  void clear_has_method_index();
  void set_has_compress_type();
  void clear_has_compress_type();
  void set_has_protocol_type();
  void clear_has_protocol_type();
  void set_has_attachment_size();
  void clear_has_attachment_size();
  void set_has_authentication_data();
  void clear_has_authentication_data();
  void set_has_user_data();
  void clear_has_user_data();
  void set_has_nshead();
  void clear_has_nshead();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr service_name_;
  ::google::protobuf::internal::ArenaStringPtr method_name_;
  ::google::protobuf::internal::ArenaStringPtr authentication_data_;
  ::google::protobuf::internal::ArenaStringPtr user_data_;
  ::google::protobuf::internal::ArenaStringPtr nshead_;
  ::google::protobuf::int32 method_index_;
  int compress_type_;
  int protocol_type_;
  ::google::protobuf::int32 attachment_size_;
  friend struct ::protobuf_brpc_2frpc_5fdump_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// RpcDumpMeta

// optional string service_name = 1;
inline bool RpcDumpMeta::has_service_name() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void RpcDumpMeta::set_has_service_name() {
  _has_bits_[0] |= 0x00000001u;
}
inline void RpcDumpMeta::clear_has_service_name() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void RpcDumpMeta::clear_service_name() {
  service_name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_service_name();
}
inline const ::std::string& RpcDumpMeta::service_name() const {
  // @@protoc_insertion_point(field_get:brpc.RpcDumpMeta.service_name)
  return service_name_.GetNoArena();
}
inline void RpcDumpMeta::set_service_name(const ::std::string& value) {
  set_has_service_name();
  service_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:brpc.RpcDumpMeta.service_name)
}
#if LANG_CXX11
inline void RpcDumpMeta::set_service_name(::std::string&& value) {
  set_has_service_name();
  service_name_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:brpc.RpcDumpMeta.service_name)
}
#endif
inline void RpcDumpMeta::set_service_name(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_service_name();
  service_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:brpc.RpcDumpMeta.service_name)
}
inline void RpcDumpMeta::set_service_name(const char* value, size_t size) {
  set_has_service_name();
  service_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:brpc.RpcDumpMeta.service_name)
}
inline ::std::string* RpcDumpMeta::mutable_service_name() {
  set_has_service_name();
  // @@protoc_insertion_point(field_mutable:brpc.RpcDumpMeta.service_name)
  return service_name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcDumpMeta::release_service_name() {
  // @@protoc_insertion_point(field_release:brpc.RpcDumpMeta.service_name)
  if (!has_service_name()) {
    return NULL;
  }
  clear_has_service_name();
  return service_name_.ReleaseNonDefaultNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcDumpMeta::set_allocated_service_name(::std::string* service_name) {
  if (service_name != NULL) {
    set_has_service_name();
  } else {
    clear_has_service_name();
  }
  service_name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), service_name);
  // @@protoc_insertion_point(field_set_allocated:brpc.RpcDumpMeta.service_name)
}

// optional string method_name = 2;
inline bool RpcDumpMeta::has_method_name() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void RpcDumpMeta::set_has_method_name() {
  _has_bits_[0] |= 0x00000002u;
}
inline void RpcDumpMeta::clear_has_method_name() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void RpcDumpMeta::clear_method_name() {
  method_name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_method_name();
}
inline const ::std::string& RpcDumpMeta::method_name() const {
  // @@protoc_insertion_point(field_get:brpc.RpcDumpMeta.method_name)
  return method_name_.GetNoArena();
}
inline void RpcDumpMeta::set_method_name(const ::std::string& value) {
  set_has_method_name();
  method_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:brpc.RpcDumpMeta.method_name)
}
#if LANG_CXX11
inline void RpcDumpMeta::set_method_name(::std::string&& value) {
  set_has_method_name();
  method_name_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:brpc.RpcDumpMeta.method_name)
}
#endif
inline void RpcDumpMeta::set_method_name(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_method_name();
  method_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:brpc.RpcDumpMeta.method_name)
}
inline void RpcDumpMeta::set_method_name(const char* value, size_t size) {
  set_has_method_name();
  method_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:brpc.RpcDumpMeta.method_name)
}
inline ::std::string* RpcDumpMeta::mutable_method_name() {
  set_has_method_name();
  // @@protoc_insertion_point(field_mutable:brpc.RpcDumpMeta.method_name)
  return method_name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcDumpMeta::release_method_name() {
  // @@protoc_insertion_point(field_release:brpc.RpcDumpMeta.method_name)
  if (!has_method_name()) {
    return NULL;
  }
  clear_has_method_name();
  return method_name_.ReleaseNonDefaultNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcDumpMeta::set_allocated_method_name(::std::string* method_name) {
  if (method_name != NULL) {
    set_has_method_name();
  } else {
    clear_has_method_name();
  }
  method_name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), method_name);
  // @@protoc_insertion_point(field_set_allocated:brpc.RpcDumpMeta.method_name)
}

// optional int32 method_index = 3;
inline bool RpcDumpMeta::has_method_index() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void RpcDumpMeta::set_has_method_index() {
  _has_bits_[0] |= 0x00000020u;
}
inline void RpcDumpMeta::clear_has_method_index() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void RpcDumpMeta::clear_method_index() {
  method_index_ = 0;
  clear_has_method_index();
}
inline ::google::protobuf::int32 RpcDumpMeta::method_index() const {
  // @@protoc_insertion_point(field_get:brpc.RpcDumpMeta.method_index)
  return method_index_;
}
inline void RpcDumpMeta::set_method_index(::google::protobuf::int32 value) {
  set_has_method_index();
  method_index_ = value;
  // @@protoc_insertion_point(field_set:brpc.RpcDumpMeta.method_index)
}

// optional .brpc.CompressType compress_type = 4;
inline bool RpcDumpMeta::has_compress_type() const {
  return (_has_bits_[0] & 0x00000040u) != 0;
}
inline void RpcDumpMeta::set_has_compress_type() {
  _has_bits_[0] |= 0x00000040u;
}
inline void RpcDumpMeta::clear_has_compress_type() {
  _has_bits_[0] &= ~0x00000040u;
}
inline void RpcDumpMeta::clear_compress_type() {
  compress_type_ = 0;
  clear_has_compress_type();
}
inline ::brpc::CompressType RpcDumpMeta::compress_type() const {
  // @@protoc_insertion_point(field_get:brpc.RpcDumpMeta.compress_type)
  return static_cast< ::brpc::CompressType >(compress_type_);
}
inline void RpcDumpMeta::set_compress_type(::brpc::CompressType value) {
  assert(::brpc::CompressType_IsValid(value));
  set_has_compress_type();
  compress_type_ = value;
  // @@protoc_insertion_point(field_set:brpc.RpcDumpMeta.compress_type)
}

// optional .brpc.ProtocolType protocol_type = 5;
inline bool RpcDumpMeta::has_protocol_type() const {
  return (_has_bits_[0] & 0x00000080u) != 0;
}
inline void RpcDumpMeta::set_has_protocol_type() {
  _has_bits_[0] |= 0x00000080u;
}
inline void RpcDumpMeta::clear_has_protocol_type() {
  _has_bits_[0] &= ~0x00000080u;
}
inline void RpcDumpMeta::clear_protocol_type() {
  protocol_type_ = 0;
  clear_has_protocol_type();
}
inline ::brpc::ProtocolType RpcDumpMeta::protocol_type() const {
  // @@protoc_insertion_point(field_get:brpc.RpcDumpMeta.protocol_type)
  return static_cast< ::brpc::ProtocolType >(protocol_type_);
}
inline void RpcDumpMeta::set_protocol_type(::brpc::ProtocolType value) {
  assert(::brpc::ProtocolType_IsValid(value));
  set_has_protocol_type();
  protocol_type_ = value;
  // @@protoc_insertion_point(field_set:brpc.RpcDumpMeta.protocol_type)
}

// optional int32 attachment_size = 6;
inline bool RpcDumpMeta::has_attachment_size() const {
  return (_has_bits_[0] & 0x00000100u) != 0;
}
inline void RpcDumpMeta::set_has_attachment_size() {
  _has_bits_[0] |= 0x00000100u;
}
inline void RpcDumpMeta::clear_has_attachment_size() {
  _has_bits_[0] &= ~0x00000100u;
}
inline void RpcDumpMeta::clear_attachment_size() {
  attachment_size_ = 0;
  clear_has_attachment_size();
}
inline ::google::protobuf::int32 RpcDumpMeta::attachment_size() const {
  // @@protoc_insertion_point(field_get:brpc.RpcDumpMeta.attachment_size)
  return attachment_size_;
}
inline void RpcDumpMeta::set_attachment_size(::google::protobuf::int32 value) {
  set_has_attachment_size();
  attachment_size_ = value;
  // @@protoc_insertion_point(field_set:brpc.RpcDumpMeta.attachment_size)
}

// optional bytes authentication_data = 7;
inline bool RpcDumpMeta::has_authentication_data() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void RpcDumpMeta::set_has_authentication_data() {
  _has_bits_[0] |= 0x00000004u;
}
inline void RpcDumpMeta::clear_has_authentication_data() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void RpcDumpMeta::clear_authentication_data() {
  authentication_data_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_authentication_data();
}
inline const ::std::string& RpcDumpMeta::authentication_data() const {
  // @@protoc_insertion_point(field_get:brpc.RpcDumpMeta.authentication_data)
  return authentication_data_.GetNoArena();
}
inline void RpcDumpMeta::set_authentication_data(const ::std::string& value) {
  set_has_authentication_data();
  authentication_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:brpc.RpcDumpMeta.authentication_data)
}
#if LANG_CXX11
inline void RpcDumpMeta::set_authentication_data(::std::string&& value) {
  set_has_authentication_data();
  authentication_data_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:brpc.RpcDumpMeta.authentication_data)
}
#endif
inline void RpcDumpMeta::set_authentication_data(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_authentication_data();
  authentication_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:brpc.RpcDumpMeta.authentication_data)
}
inline void RpcDumpMeta::set_authentication_data(const void* value, size_t size) {
  set_has_authentication_data();
  authentication_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:brpc.RpcDumpMeta.authentication_data)
}
inline ::std::string* RpcDumpMeta::mutable_authentication_data() {
  set_has_authentication_data();
  // @@protoc_insertion_point(field_mutable:brpc.RpcDumpMeta.authentication_data)
  return authentication_data_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcDumpMeta::release_authentication_data() {
  // @@protoc_insertion_point(field_release:brpc.RpcDumpMeta.authentication_data)
  if (!has_authentication_data()) {
    return NULL;
  }
  clear_has_authentication_data();
  return authentication_data_.ReleaseNonDefaultNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcDumpMeta::set_allocated_authentication_data(::std::string* authentication_data) {
  if (authentication_data != NULL) {
    set_has_authentication_data();
  } else {
    clear_has_authentication_data();
  }
  authentication_data_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), authentication_data);
  // @@protoc_insertion_point(field_set_allocated:brpc.RpcDumpMeta.authentication_data)
}

// optional bytes user_data = 8;
inline bool RpcDumpMeta::has_user_data() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void RpcDumpMeta::set_has_user_data() {
  _has_bits_[0] |= 0x00000008u;
}
inline void RpcDumpMeta::clear_has_user_data() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void RpcDumpMeta::clear_user_data() {
  user_data_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_user_data();
}
inline const ::std::string& RpcDumpMeta::user_data() const {
  // @@protoc_insertion_point(field_get:brpc.RpcDumpMeta.user_data)
  return user_data_.GetNoArena();
}
inline void RpcDumpMeta::set_user_data(const ::std::string& value) {
  set_has_user_data();
  user_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:brpc.RpcDumpMeta.user_data)
}
#if LANG_CXX11
inline void RpcDumpMeta::set_user_data(::std::string&& value) {
  set_has_user_data();
  user_data_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:brpc.RpcDumpMeta.user_data)
}
#endif
inline void RpcDumpMeta::set_user_data(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_user_data();
  user_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:brpc.RpcDumpMeta.user_data)
}
inline void RpcDumpMeta::set_user_data(const void* value, size_t size) {
  set_has_user_data();
  user_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:brpc.RpcDumpMeta.user_data)
}
inline ::std::string* RpcDumpMeta::mutable_user_data() {
  set_has_user_data();
  // @@protoc_insertion_point(field_mutable:brpc.RpcDumpMeta.user_data)
  return user_data_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcDumpMeta::release_user_data() {
  // @@protoc_insertion_point(field_release:brpc.RpcDumpMeta.user_data)
  if (!has_user_data()) {
    return NULL;
  }
  clear_has_user_data();
  return user_data_.ReleaseNonDefaultNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcDumpMeta::set_allocated_user_data(::std::string* user_data) {
  if (user_data != NULL) {
    set_has_user_data();
  } else {
    clear_has_user_data();
  }
  user_data_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), user_data);
  // @@protoc_insertion_point(field_set_allocated:brpc.RpcDumpMeta.user_data)
}

// optional bytes nshead = 9;
inline bool RpcDumpMeta::has_nshead() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void RpcDumpMeta::set_has_nshead() {
  _has_bits_[0] |= 0x00000010u;
}
inline void RpcDumpMeta::clear_has_nshead() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void RpcDumpMeta::clear_nshead() {
  nshead_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_nshead();
}
inline const ::std::string& RpcDumpMeta::nshead() const {
  // @@protoc_insertion_point(field_get:brpc.RpcDumpMeta.nshead)
  return nshead_.GetNoArena();
}
inline void RpcDumpMeta::set_nshead(const ::std::string& value) {
  set_has_nshead();
  nshead_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:brpc.RpcDumpMeta.nshead)
}
#if LANG_CXX11
inline void RpcDumpMeta::set_nshead(::std::string&& value) {
  set_has_nshead();
  nshead_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:brpc.RpcDumpMeta.nshead)
}
#endif
inline void RpcDumpMeta::set_nshead(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_nshead();
  nshead_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:brpc.RpcDumpMeta.nshead)
}
inline void RpcDumpMeta::set_nshead(const void* value, size_t size) {
  set_has_nshead();
  nshead_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:brpc.RpcDumpMeta.nshead)
}
inline ::std::string* RpcDumpMeta::mutable_nshead() {
  set_has_nshead();
  // @@protoc_insertion_point(field_mutable:brpc.RpcDumpMeta.nshead)
  return nshead_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcDumpMeta::release_nshead() {
  // @@protoc_insertion_point(field_release:brpc.RpcDumpMeta.nshead)
  if (!has_nshead()) {
    return NULL;
  }
  clear_has_nshead();
  return nshead_.ReleaseNonDefaultNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcDumpMeta::set_allocated_nshead(::std::string* nshead) {
  if (nshead != NULL) {
    set_has_nshead();
  } else {
    clear_has_nshead();
  }
  nshead_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), nshead);
  // @@protoc_insertion_point(field_set_allocated:brpc.RpcDumpMeta.nshead)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace brpc

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_brpc_2frpc_5fdump_2eproto
