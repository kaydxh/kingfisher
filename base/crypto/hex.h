#ifndef CRYPTO_HEX_H
#define CRYPTO_HEX_H

#include <cctype>
namespace kingfisher {
namespace crypto {

/* Converts an integer value to its hex character*/
char ToHex(char c);

/* Converts a hex character to its integer value */
char FromHex(char c);
}
}

#endif
