#ifndef KINNGFISHER_BASE_ENCODING_BASE64_H_
#define KINNGFISHER_BASE_ENCODING_BASE64_H_

#include <string>

namespace kingfisher {
namespace encoding {

std::string Base64Encode(const std::string& v);
int Base64Decode(const std::string& input, std::string& output);

}  // namespace encoding
}  // namespace kingfisher

#endif
