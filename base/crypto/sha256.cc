#include "sha256.h"

#ifdef ENABLE_CRYPTO_OPENSSL

#include <string.h>

#include <iostream>
#include <memory>

#include "openssl/crypto.h"
namespace kingfisher {
namespace crypto {

SHA256::SHA256() {
  // CRYPTO_library_init();
  SHA256_Init(&ctx_);
}

SHA256::~SHA256() { OPENSSL_cleanse(&ctx_, sizeof(ctx_)); }

SHA256::SHA256(const SHA256& other) : CryptoBase(other) {
  memcpy(&ctx_, &other.ctx_, sizeof(ctx_));
}

void SHA256::Update(const void* input, size_t len) {
  SHA256_Update(&ctx_, static_cast<const unsigned char*>(input), len);
}

void SHA256::Finish(void* output, size_t len) {
  if (len < SHA256_DIGEST_LENGTH) {
    unsigned char buffer[SHA256_DIGEST_LENGTH] = {0};  // SHA256_DIGEST_LENGTH
    SHA256_Final(buffer, &ctx_);
    // copy the temporary buffer out, truncating as needed.
    memcpy(output, buffer, len);
    return;
  }

  // normal get this line
  SHA256_Final((unsigned char*)output, &ctx_);
  return;
}

size_t SHA256::GetHashLength() const { return SHA256_DIGEST_LENGTH; }

std::string Sha256SumString(const std::string& str) {
  std::shared_ptr<CryptoBase> sha256(new SHA256());
  sha256->Update(str.data(), str.length());

  std::string hash(SHA256_DIGEST_LENGTH, 0);
  sha256->Finish((void*)(hash.data()), SHA256_DIGEST_LENGTH);
  /*
  unsigned char hash[SHA256_DIGEST_LENGTH] = {0};
  sha256->Finish((void*)(hash), SHA256_DIGEST_LENGTH);
  */

  char output[2 * SHA256_DIGEST_LENGTH + 1] = {0};
  for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    // need convert (unsigned char to print hex, or it will overflow
    sprintf(output + i * 2, "%02x", (unsigned char)(hash[i]));
  }
  return std::string(output);
}

}  // namespace crypto
}  // namespace kingfisher

#endif
