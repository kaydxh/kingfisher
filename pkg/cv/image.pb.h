// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: pkg/cv/image.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_pkg_2fcv_2fimage_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_pkg_2fcv_2fimage_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3017000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3017003 < PROTOBUF_MIN_PROTOC_VERSION
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
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/duration.pb.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_pkg_2fcv_2fimage_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_pkg_2fcv_2fimage_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[2]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_pkg_2fcv_2fimage_2eproto;
namespace kingfisher {
namespace kcv {
class DecodeOptions;
struct DecodeOptionsDefaultTypeInternal;
extern DecodeOptionsDefaultTypeInternal _DecodeOptions_default_instance_;
class Rect;
struct RectDefaultTypeInternal;
extern RectDefaultTypeInternal _Rect_default_instance_;
}  // namespace kcv
}  // namespace kingfisher
PROTOBUF_NAMESPACE_OPEN
template<> ::kingfisher::kcv::DecodeOptions* Arena::CreateMaybeMessage<::kingfisher::kcv::DecodeOptions>(Arena*);
template<> ::kingfisher::kcv::Rect* Arena::CreateMaybeMessage<::kingfisher::kcv::Rect>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace kingfisher {
namespace kcv {

enum ColorSpace : int {
  UnknownColorSpace = 0,
  BGRColorSpace = 1,
  BGRAColorSpace = 2,
  GRAYColorSpace = 3,
  GRAYAColorSpace = 4,
  ColorSpace_INT_MIN_SENTINEL_DO_NOT_USE_ = std::numeric_limits<::PROTOBUF_NAMESPACE_ID::int32>::min(),
  ColorSpace_INT_MAX_SENTINEL_DO_NOT_USE_ = std::numeric_limits<::PROTOBUF_NAMESPACE_ID::int32>::max()
};
bool ColorSpace_IsValid(int value);
constexpr ColorSpace ColorSpace_MIN = UnknownColorSpace;
constexpr ColorSpace ColorSpace_MAX = GRAYAColorSpace;
constexpr int ColorSpace_ARRAYSIZE = ColorSpace_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* ColorSpace_descriptor();
template<typename T>
inline const std::string& ColorSpace_Name(T enum_t_value) {
  static_assert(::std::is_same<T, ColorSpace>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function ColorSpace_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    ColorSpace_descriptor(), enum_t_value);
}
inline bool ColorSpace_Parse(
    ::PROTOBUF_NAMESPACE_ID::ConstStringParam name, ColorSpace* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<ColorSpace>(
    ColorSpace_descriptor(), name, value);
}
// ===================================================================

class DecodeOptions final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:kingfisher.kcv.DecodeOptions) */ {
 public:
  inline DecodeOptions() : DecodeOptions(nullptr) {}
  ~DecodeOptions() override;
  explicit constexpr DecodeOptions(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  DecodeOptions(const DecodeOptions& from);
  DecodeOptions(DecodeOptions&& from) noexcept
    : DecodeOptions() {
    *this = ::std::move(from);
  }

  inline DecodeOptions& operator=(const DecodeOptions& from) {
    CopyFrom(from);
    return *this;
  }
  inline DecodeOptions& operator=(DecodeOptions&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const DecodeOptions& default_instance() {
    return *internal_default_instance();
  }
  static inline const DecodeOptions* internal_default_instance() {
    return reinterpret_cast<const DecodeOptions*>(
               &_DecodeOptions_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(DecodeOptions& a, DecodeOptions& b) {
    a.Swap(&b);
  }
  inline void Swap(DecodeOptions* other) {
    if (other == this) return;
    if (GetOwningArena() == other->GetOwningArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(DecodeOptions* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline DecodeOptions* New() const final {
    return new DecodeOptions();
  }

  DecodeOptions* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<DecodeOptions>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const DecodeOptions& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const DecodeOptions& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to, const ::PROTOBUF_NAMESPACE_ID::Message&from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(DecodeOptions* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "kingfisher.kcv.DecodeOptions";
  }
  protected:
  explicit DecodeOptions(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kTargetColorSpaceFieldNumber = 1,
    kAutoOrientFieldNumber = 2,
  };
  // .kingfisher.kcv.ColorSpace targetColorSpace = 1;
  void clear_targetcolorspace();
  ::kingfisher::kcv::ColorSpace targetcolorspace() const;
  void set_targetcolorspace(::kingfisher::kcv::ColorSpace value);
  private:
  ::kingfisher::kcv::ColorSpace _internal_targetcolorspace() const;
  void _internal_set_targetcolorspace(::kingfisher::kcv::ColorSpace value);
  public:

  // bool auto_orient = 2;
  void clear_auto_orient();
  bool auto_orient() const;
  void set_auto_orient(bool value);
  private:
  bool _internal_auto_orient() const;
  void _internal_set_auto_orient(bool value);
  public:

  // @@protoc_insertion_point(class_scope:kingfisher.kcv.DecodeOptions)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  int targetcolorspace_;
  bool auto_orient_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_pkg_2fcv_2fimage_2eproto;
};
// -------------------------------------------------------------------

class Rect final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:kingfisher.kcv.Rect) */ {
 public:
  inline Rect() : Rect(nullptr) {}
  ~Rect() override;
  explicit constexpr Rect(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  Rect(const Rect& from);
  Rect(Rect&& from) noexcept
    : Rect() {
    *this = ::std::move(from);
  }

  inline Rect& operator=(const Rect& from) {
    CopyFrom(from);
    return *this;
  }
  inline Rect& operator=(Rect&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const Rect& default_instance() {
    return *internal_default_instance();
  }
  static inline const Rect* internal_default_instance() {
    return reinterpret_cast<const Rect*>(
               &_Rect_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(Rect& a, Rect& b) {
    a.Swap(&b);
  }
  inline void Swap(Rect* other) {
    if (other == this) return;
    if (GetOwningArena() == other->GetOwningArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(Rect* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline Rect* New() const final {
    return new Rect();
  }

  Rect* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<Rect>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const Rect& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const Rect& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to, const ::PROTOBUF_NAMESPACE_ID::Message&from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Rect* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "kingfisher.kcv.Rect";
  }
  protected:
  explicit Rect(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kXFieldNumber = 1,
    kYFieldNumber = 2,
    kHeightFieldNumber = 3,
    kWidthFieldNumber = 4,
  };
  // int32 x = 1;
  void clear_x();
  ::PROTOBUF_NAMESPACE_ID::int32 x() const;
  void set_x(::PROTOBUF_NAMESPACE_ID::int32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int32 _internal_x() const;
  void _internal_set_x(::PROTOBUF_NAMESPACE_ID::int32 value);
  public:

  // int32 y = 2;
  void clear_y();
  ::PROTOBUF_NAMESPACE_ID::int32 y() const;
  void set_y(::PROTOBUF_NAMESPACE_ID::int32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int32 _internal_y() const;
  void _internal_set_y(::PROTOBUF_NAMESPACE_ID::int32 value);
  public:

  // int32 height = 3;
  void clear_height();
  ::PROTOBUF_NAMESPACE_ID::int32 height() const;
  void set_height(::PROTOBUF_NAMESPACE_ID::int32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int32 _internal_height() const;
  void _internal_set_height(::PROTOBUF_NAMESPACE_ID::int32 value);
  public:

  // int32 width = 4;
  void clear_width();
  ::PROTOBUF_NAMESPACE_ID::int32 width() const;
  void set_width(::PROTOBUF_NAMESPACE_ID::int32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int32 _internal_width() const;
  void _internal_set_width(::PROTOBUF_NAMESPACE_ID::int32 value);
  public:

  // @@protoc_insertion_point(class_scope:kingfisher.kcv.Rect)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::int32 x_;
  ::PROTOBUF_NAMESPACE_ID::int32 y_;
  ::PROTOBUF_NAMESPACE_ID::int32 height_;
  ::PROTOBUF_NAMESPACE_ID::int32 width_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_pkg_2fcv_2fimage_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// DecodeOptions

// .kingfisher.kcv.ColorSpace targetColorSpace = 1;
inline void DecodeOptions::clear_targetcolorspace() {
  targetcolorspace_ = 0;
}
inline ::kingfisher::kcv::ColorSpace DecodeOptions::_internal_targetcolorspace() const {
  return static_cast< ::kingfisher::kcv::ColorSpace >(targetcolorspace_);
}
inline ::kingfisher::kcv::ColorSpace DecodeOptions::targetcolorspace() const {
  // @@protoc_insertion_point(field_get:kingfisher.kcv.DecodeOptions.targetColorSpace)
  return _internal_targetcolorspace();
}
inline void DecodeOptions::_internal_set_targetcolorspace(::kingfisher::kcv::ColorSpace value) {
  
  targetcolorspace_ = value;
}
inline void DecodeOptions::set_targetcolorspace(::kingfisher::kcv::ColorSpace value) {
  _internal_set_targetcolorspace(value);
  // @@protoc_insertion_point(field_set:kingfisher.kcv.DecodeOptions.targetColorSpace)
}

// bool auto_orient = 2;
inline void DecodeOptions::clear_auto_orient() {
  auto_orient_ = false;
}
inline bool DecodeOptions::_internal_auto_orient() const {
  return auto_orient_;
}
inline bool DecodeOptions::auto_orient() const {
  // @@protoc_insertion_point(field_get:kingfisher.kcv.DecodeOptions.auto_orient)
  return _internal_auto_orient();
}
inline void DecodeOptions::_internal_set_auto_orient(bool value) {
  
  auto_orient_ = value;
}
inline void DecodeOptions::set_auto_orient(bool value) {
  _internal_set_auto_orient(value);
  // @@protoc_insertion_point(field_set:kingfisher.kcv.DecodeOptions.auto_orient)
}

// -------------------------------------------------------------------

// Rect

// int32 x = 1;
inline void Rect::clear_x() {
  x_ = 0;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 Rect::_internal_x() const {
  return x_;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 Rect::x() const {
  // @@protoc_insertion_point(field_get:kingfisher.kcv.Rect.x)
  return _internal_x();
}
inline void Rect::_internal_set_x(::PROTOBUF_NAMESPACE_ID::int32 value) {
  
  x_ = value;
}
inline void Rect::set_x(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _internal_set_x(value);
  // @@protoc_insertion_point(field_set:kingfisher.kcv.Rect.x)
}

// int32 y = 2;
inline void Rect::clear_y() {
  y_ = 0;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 Rect::_internal_y() const {
  return y_;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 Rect::y() const {
  // @@protoc_insertion_point(field_get:kingfisher.kcv.Rect.y)
  return _internal_y();
}
inline void Rect::_internal_set_y(::PROTOBUF_NAMESPACE_ID::int32 value) {
  
  y_ = value;
}
inline void Rect::set_y(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _internal_set_y(value);
  // @@protoc_insertion_point(field_set:kingfisher.kcv.Rect.y)
}

// int32 height = 3;
inline void Rect::clear_height() {
  height_ = 0;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 Rect::_internal_height() const {
  return height_;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 Rect::height() const {
  // @@protoc_insertion_point(field_get:kingfisher.kcv.Rect.height)
  return _internal_height();
}
inline void Rect::_internal_set_height(::PROTOBUF_NAMESPACE_ID::int32 value) {
  
  height_ = value;
}
inline void Rect::set_height(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _internal_set_height(value);
  // @@protoc_insertion_point(field_set:kingfisher.kcv.Rect.height)
}

// int32 width = 4;
inline void Rect::clear_width() {
  width_ = 0;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 Rect::_internal_width() const {
  return width_;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 Rect::width() const {
  // @@protoc_insertion_point(field_get:kingfisher.kcv.Rect.width)
  return _internal_width();
}
inline void Rect::_internal_set_width(::PROTOBUF_NAMESPACE_ID::int32 value) {
  
  width_ = value;
}
inline void Rect::set_width(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _internal_set_width(value);
  // @@protoc_insertion_point(field_set:kingfisher.kcv.Rect.width)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace kcv
}  // namespace kingfisher

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::kingfisher::kcv::ColorSpace> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::kingfisher::kcv::ColorSpace>() {
  return ::kingfisher::kcv::ColorSpace_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_pkg_2fcv_2fimage_2eproto