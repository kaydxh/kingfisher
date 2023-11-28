#ifndef KINNGFISHER_BASE_ENCODING_UTF8_H_
#define KINNGFISHER_BASE_ENCODING_UTF8_H_

#include <cstddef>

namespace kingfisher {
namespace encoding {

size_t EncodeUTF8Char(char *buffer, char32_t utf8_char);

}
}  // namespace kingfisher

#endif
