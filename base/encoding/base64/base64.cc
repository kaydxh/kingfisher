#include "base64.h"

#include "modp_b64.h"

namespace kingfisher {
namespace encoding {

std::string Base64Encode(const std::string& v) {
  std::string result;
  result.resize(modp_b64_encode_len(v.size()));  // makes room for null byte

  // modp_b64_encode_len() returns at least 1, so temp[0] is safe to use.
  size_t output_size = modp_b64_encode(&(result[0]), v.data(), v.size());

  result.resize(output_size);  // strips off null byte
  return result;
}

int Base64Decode(const std::string& input, std::string& output) {
  output.resize(modp_b64_decode_len(input.size()));

  // does not null terminate result since result is binary data!
  size_t input_size = input.size();
  size_t output_size = modp_b64_decode(&(output[0]), input.data(), input_size);
  if (output_size == MODP_B64_ERROR) {
    return false;
  }

  output.resize(output_size);
  return true;
}

}  // namespace encoding
}  // namespace kingfisher
