#ifndef KINGFISHER_BASE_NET_SOCKET_ENDIAN_H_
#define KINGFISHER_BASE_NET_SOCKET_ENDIAN_H_
#include <endian.h>

#include <cstdint>
#include <type_traits>
namespace kingfisher {

#include <stdint.h>

namespace net {
namespace sockets {

uint64_t hton64(uint64_t h) { return htobe64(h); }
uint32_t hton32(uint32_t h) { return htobe32(h); }
uint16_t hton16(uint16_t h) { return htobe16(h); }

uint64_t ntoh64(uint64_t n) { return be64toh(n); }
uint32_t ntoh32(uint32_t n) { return be32toh(n); }
uint16_t ntoh16(uint16_t n) { return be16toh(n); }

template <typename T>
T Hton(T h) {
  static_assert(std::is_integral<T>::value, "must be integer");
  auto sz = sizeof(T);
  switch (sz) {
    case sizeof(uint8_t): {
      return h;
    }
    case sizeof(uint16_t): {
      return hton16(h);
    }
    case sizeof(uint32_t): {
      return hton32(h);
    }
    case sizeof(uint64_t): {
      return hton64(h);
    }

    default:
      // must not arrived this
      return h;
  }
}

template <typename T>
T Ntoh(T n) {
  static_assert(std::is_integral<T>::value, "must be integer");
  auto sz = sizeof(T);
  switch (sz) {
    case sizeof(uint8_t): {
      return n;
    }
    case sizeof(uint16_t): {
      return ntoh16(n);
    }
    case sizeof(uint32_t): {
      return ntoh32(n);
    }
    case sizeof(uint64_t): {
      return ntoh64(n);
    }

    default:
      // must not arrived this
      return n;
  }
}

}  // namespace sockets
}  // namespace net
}  // namespace kingfisher

#endif
