#ifndef CRYPTO_SHA256_H
#define CRYPTO_SHA256_H

#ifdef ENABLE_CRYPTO_OPENSSL
#include <string>

#include "crypto/crypto_base.h"
#include "openssl/crypto.h"
#include "openssl/sha.h"

namespace kingfisher {
namespace crypto {
class SHA256 : public CryptoBase {
 public:
  SHA256();
  ~SHA256();
  SHA256(const SHA256& other);

 public:
  void Update(const void* input, size_t len) override;
  void Finish(void* output, size_t len) override;
  size_t GetHashLength() const override;

 private:
  SHA256_CTX ctx_;
};

std::string Sha256SumString(const std::string& str);

}  // namespace crypto
}  // namespace kingfisher
#endif
#endif
