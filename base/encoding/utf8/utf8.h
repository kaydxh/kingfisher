#ifndef KINNGFISHER_BASE_ENCODING_UTF8_H_
#define KINNGFISHER_BASE_ENCODING_UTF8_H_

#include <cstddef>

namespace kingfisher {
namespace encoding {

// 将一个Unicode字符（UTF-32编码）转换为UTF-8编码，并将编码后的字节序列存储到提供的缓冲区中。同时，该函数返回转换后的UTF-8字节序列的长度
size_t EncodeUTF8Char(char *buffer, char32_t utf8_char);

}  // namespace encoding
}  // namespace kingfisher

#endif
