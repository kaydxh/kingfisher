#ifndef CRYPTO_CRYPTO_BASE_H
#define CRYPTO_CRYPTO_BASE_H

#include <memory>

namespace kingfisher {
namespace crypto {
class CryptoBase {
 public:
  enum Algorithm {
    SHA256,
  };

  // CryptoBase(const CryptoBase &) = delete;
  CryptoBase(const CryptoBase &){};
  CryptoBase &operator=(const CryptoBase &) = delete;

  virtual ~CryptoBase() {}
  virtual void Update(const void *input, size_t len) = 0;
  virtual void Finish(void *output, size_t len) = 0;
  virtual size_t GetHashLength() const = 0;
  // Create a clone of this SecureHash. The returned clone and this both
  // represent the same hash state. But from this point on, calling
  // Update()/Finish() on either doesn't affect the state of the other.
  //  virtual std::unique_ptr<CryptoBase> Clone() const = 0;
  //
 protected:
  CryptoBase() {}
};

}  // namespace crypto
}  // namespace kingfisher
#endif
