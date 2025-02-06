
#include "strings/strings.h"

#include <assert.h>
#include <stdarg.h>  //va_start

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <cstring>
#include <exception>
#include <locale>
#include <sstream>

#include "encoding/utf8/utf8.h"

namespace kingfisher {
namespace strings {

bool IsNumber(const std::string &s) {
  if (s.empty()) {
    return false;
  }

  for (size_t i = 0; i < s.length(); ++i) {
    if (!std::isdigit(s[i])) {
      if (i == 0 && s.length() > 1) {
        if (s[i] == '-' || s[i] == '+') {
          continue;
        }
      }
      return false;
    }
  }

  return true;
}

int ParseInt64(int64_t &result, const std::string &s, int base) {
  if (!IsNumber(s)) {
    return -1;
  }

  result = std::stoll(s);
  return 0;
}

int ParseUint64(uint64_t &result, const std::string &s, int base) {
  if (!IsNumber(s)) {
    return -1;
  }

  result = std::stoull(s);
  return 0;
}

std::vector<std::string> Split(const std::string &s, const char delim) {
#if 0
  std::string word;
  std::stringstream ss(s);
  std::vector<std::string> words;

  while (std::getline(ss, word, delim)) {
    words.push_back(word);
  }
  return words;
#endif
  std::vector<std::string> results;
  auto last(0);
  auto found(s.find_first_of(delim));
  while (std::string::npos != found) {
    auto r(s.substr(last, found - last));
    last = found + 1;
    found = s.find_first_of(delim, last);
    if (!r.empty()) {
      results.push_back(r);
    }
  }
  auto r(s.substr(last));
  if (!r.empty()) {
    results.push_back(r);
  }

  return results;
}

std::vector<int64_t> SplitToInt64(const std::string &s, const char delim) {
  std::vector<int64_t> results;
  auto strs = Split(s, delim);
  for (auto str : strs) {
    int64_t value = 0;
    int ret = ParseInt64(value, str, 10);
    if (ret != 0) {
      return {};
    }
    results.push_back(value);
  }

  return results;
}

std::string Join(const std::vector<std::string> &elems,
                 const std::string &sep) {
  return Join<std::string>(elems, sep);
  /*
int sz = elems.size();
if (sz == 0) {
  return "";
}
if (sz == 1) {
  return elems[0];
}

std::string s = elems[0];
for (auto i = 1; i < sz; ++i) {
  s.append(sep);
  s.append(elems[i]);
}

return s;
*/
}

bool HasPrefix(const std::string &s, const std::string &prefix,
               bool case_sensitive) {
  if (s.length() < prefix.length()) {
    return false;
  }

  std::string s_lower = s;
  std::string prefix_lower = prefix;
  if (!case_sensitive) {
    s_lower = ToLower(s);
    prefix_lower = ToLower(prefix);
  }

  auto pos = s_lower.find(prefix_lower);
  return 0 == pos;
}

bool HasSuffix(const std::string &s, const std::string &suffix,
               bool case_sensitive) {
  if (s.length() < suffix.length()) {
    return false;
  }

  std::string s_lower = s;
  std::string suffix_lower = suffix;
  if (!case_sensitive) {
    s_lower = ToLower(s);
    suffix_lower = ToLower(suffix);
  }

  auto pos = s_lower.rfind(suffix_lower);
  return pos == s.length() - suffix.length();
}

std::string ToLower(const std::string &s) {
  std::string lower;
  std::transform(s.begin(), s.end(), lower.begin(), ::tolower);
  return lower;
}

/**
 * Returns a substring with all characters the provided @toTrim returns true
 * for removed from the front of @sp.
 */
template <typename ToTrim>
std::string ltrim(std::string s, ToTrim toTrim) {
  while (!s.empty() && toTrim(s.front())) {
    s = s.substr(1);
  }

  return s;
}

std::string TrimLeft(const std::string &s, const std::string &prefix) {
  if (HasPrefix(s, prefix)) {
    return TrimLeft(s.substr(prefix.length()), prefix);
  }
  return s;
}

std::string TrimRight(const std::string &s, const std::string &suffix) {
  if (HasSuffix(s, suffix)) {
    return TrimRight(s.substr(0, s.length() - suffix.length()), suffix);
  }
  return s;
}

std::string Trim(const std::string &s) {
  if (s.empty()) {
    return s;
  }
  size_t left = 0;
  for (; left < s.size() && std::isspace(s[left]); ++left) {
  }

  size_t right = s.size() - 1;
  for (; right > left && std::isspace(s[right]); --right) {
  }

  return s.substr(left, right - left + 1);
}

bool Contains(const std::string &s, const std::string &sub) {
  auto pos = s.find_first_of(sub);
  if (pos == std::string::npos) {
    return false;
  }
  return true;
}

std::string FormatString(const char *fmt, ...) {
  if (fmt == nullptr) {
    return "";
  }

  std::string s;
  va_list va;
  va_start(va, fmt);
  size_t len = vsnprintf(nullptr, 0, fmt, va);
  if (len > 0) {
    va_list va2;
    va_start(va2, fmt);

    s.resize(len);
    char *tmp = (char *)s.c_str();
    vsnprintf(tmp, len + 1, fmt, va2);

    va_end(va2);
  }
  va_end(va);

  return s;
}

bool Unescape(std::string &dest, const std::string &source,
              bool leave_nulls_escaped) {
  dest.resize(source.size());
  char *d = dest.data();
  const char *p = source.data();
  const char *end = p + source.size();
  const char *last_byte = end - 1;

  // Small optimization for case where source = dest and there's no escaping
  while (p == d && p < end && *p != '\\') p++, d++;

  while (p < end) {
    if (*p != '\\') {
      *d++ = *p++;
    } else {
      if (++p > last_byte) {  // skip past the '\\'
        return false;
      }
      switch (*p) {
        case 'a':
          *d++ = '\a';
          break;
        case 'b':
          *d++ = '\b';
          break;
        case 'f':
          *d++ = '\f';
          break;
        case 'n':
          *d++ = '\n';
          break;
        case 'r':
          *d++ = '\r';
          break;
        case 't':
          *d++ = '\t';
          break;
        case 'v':
          *d++ = '\v';
          break;
        case '\\':
          *d++ = '\\';
          break;
        case '?':
          *d++ = '\?';
          break;  // \?  Who knew?
        case '\'':
          *d++ = '\'';
          break;
        case '"':
          *d++ = '\"';
          break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
          // octal digit: 1 to 3 digits
          const char *octal_start = p;
          unsigned int ch = static_cast<unsigned int>(*p - '0');  // digit 1
          if (p < last_byte && IsOctalDigit(p[1]))
            ch = ch * 8 + static_cast<unsigned int>(*++p - '0');  // digit 2
          if (p < last_byte && IsOctalDigit(p[1]))
            ch = ch * 8 + static_cast<unsigned int>(*++p - '0');  // digit 3
          if (ch > 0xff) {
            return false;
          }
          if ((ch == 0) && leave_nulls_escaped) {
            // Copy the escape sequence for the null character
            const size_t octal_size = static_cast<size_t>(p + 1 - octal_start);
            *d++ = '\\';
            memmove(d, octal_start, octal_size);
            d += octal_size;
            break;
          }
          *d++ = static_cast<char>(ch);
          break;
        }
        case 'x':
        case 'X': {
          if (p >= last_byte) {
            return false;
          } else if (!IsHexDigit(static_cast<unsigned char>(p[1]))) {
            return false;
          }
          unsigned int ch = 0;
          const char *hex_start = p;
          while (p < last_byte && IsHexDigit(static_cast<unsigned char>(p[1])))
            // Arbitrarily many hex digits
            ch = (ch << 4) + HexDigitToInt(*++p);
          if (ch > 0xFF) {
            // "Value of \\" + std::string(hex_start,
            // static_cast<size_t>(p + 1
            // - hex_start)) + " exceeds 0xff";

            return false;
          }
          if ((ch == 0) && leave_nulls_escaped) {
            // Copy the escape sequence for the null character
            const size_t hex_size = static_cast<size_t>(p + 1 - hex_start);
            *d++ = '\\';
            memmove(d, hex_start, hex_size);
            d += hex_size;
            break;
          }
          *d++ = static_cast<char>(ch);
          break;
        }
        case 'u': {
          // \uhhhh => convert 4 hex digits to UTF-8
          char32_t rune = 0;
          const char *hex_start = p;
          if (p + 4 >= end) {
            // error = "\\u must be followed by 4 hex digits: \\" +
            // std::string(hex_start, static_cast<size_t>(p + 1 -
            // hex_start));

            return false;
          }
          for (int i = 0; i < 4; ++i) {
            // Look one char ahead.
            if (IsHexDigit(static_cast<unsigned char>(p[1]))) {
              rune = (rune << 4) + HexDigitToInt(*++p);  // Advance p.
            } else {
              // "\\u must be followed by 4 hex digits: \\" +
              // std::string(hex_start, static_cast<size_t>(p + 1
              // - hex_start));

              return false;
            }
          }
          if ((rune == 0) && leave_nulls_escaped) {
            // Copy the escape sequence for the null character
            *d++ = '\\';
            memmove(d, hex_start, 5);  // u0000
            d += 5;
            break;
          }
          if (IsSurrogate(rune, std::string(hex_start, 5))) {
            return false;
          }
          d += encoding::EncodeUTF8Char(d, rune);
          break;
        }
        case 'U': {
          // \Uhhhhhhhh => convert 8 hex digits to UTF-8
          char32_t rune = 0;
          const char *hex_start = p;
          if (p + 8 >= end) {
            //"\\U must be followed by 8 hex digits: \\" +
            // std::string(hex_start, static_cast<size_t>(p + 1 -
            // hex_start));

            return false;
          }
          for (int i = 0; i < 8; ++i) {
            // Look one char ahead.
            if (IsHexDigit(static_cast<unsigned char>(p[1]))) {
              // Don't change rune until we're sure this
              // is within the Unicode limit, but do advance p.
              uint32_t newrune = (rune << 4) + HexDigitToInt(*++p);
              if (newrune > 0x10FFFF) {
                // "Value of \\" + std::string(hex_start,
                // static_cast<size_t>(p
                // + 1 - hex_start)) + " exceeds Unicode limit
                // (0x10FFFF)";

                return false;
              } else {
                rune = newrune;
              }
            } else {
              // "\\U must be followed by 8 hex digits: \\" +
              // std::string(hex_start, static_cast<size_t>(p + 1
              // - hex_start));

              return false;
            }
          }
          if ((rune == 0) && leave_nulls_escaped) {
            // Copy the escape sequence for the null character
            *d++ = '\\';
            memmove(d, hex_start, 9);  // U00000000
            d += 9;
            break;
          }
          if (IsSurrogate(rune, std::string(hex_start, 9))) {
            return false;
          }
          d += encoding::EncodeUTF8Char(d, rune);
          break;
        }
        default: {
          // std::string("Unknown escape sequence: \\") + *p;
          return false;
        }
      }
      p++;  // read past letter we escaped
    }
  }
  dest.erase(static_cast<size_t>(d - dest.data()));
  return true;
}

bool IsSurrogate(char32_t c, std::string src) {
  if (c >= 0xD800 && c <= 0xDFFF) {
    return true;
  }
  return false;
}

bool IsOctalDigit(char c) { return ('0' <= c) && (c <= '7'); }
bool IsHexDigit(unsigned char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
         (c >= 'a' && c <= 'f');
}

unsigned int HexDigitToInt(char c) {
  static_assert('0' == 0x30 && 'A' == 0x41 && 'a' == 0x61,
                "Character set must be ASCII.");
  assert(IsHexDigit(static_cast<unsigned char>(c)));
  unsigned int x = static_cast<unsigned char>(c);
  if (x > '9') {
    x += 9;
  }
  return x & 0xf;
}

std::string ToHexString(const std::string &input) {
  std::string output;
  static const char *const lut = "0123456789abcdef";
  output.reserve(2 * input.size());
  for (size_t i = 0; i < input.size(); i++) {
    const unsigned char c = input[i];
    output.push_back(lut[c >> 4]);
    output.push_back(lut[c & 0xf]);
  }
  return output;
}

int WStringFromBytes(std::wstring &result, const std::string &s) {
  if (s.empty()) {
    return 0;
  }
  static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

  try {
    result = converter.from_bytes(s);
  } catch (std::exception &e) {
    return -1;
  }

  return 0;
}

int WStringToBytes(std::string &result, const std::wstring &ws) {
  if (ws.empty()) {
    return 0;
  }
  static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

  try {
    result = converter.to_bytes(ws);
  } catch (std::exception &e) {
    return -1;
  }

  return 0;
}

bool IsChineseChar(const wchar_t wch) {
  return (
      // 基本汉字（CJK Unified Ideographs）范围：0x4E00 - 0x9FFF
      (wch >= 0x4E00 && wch <= 0x9FFF) ||
      // 扩展 A 区（CJK Unified Ideographs Extension A）范围：0x3400 - 0x4DBF
      (wch >= 0x3400 && wch <= 0x4DBF) ||
      // 扩展 B 区（CJK Unified Ideographs Extension B）范围：0x20000 -
      // 0x2A6DF
      (wch >= 0x20000 && wch <= 0x2A6DF) ||
      // 扩展 C 区（CJK Unified Ideographs Extension C）范围：0x2A700 -
      // 0x2B73F
      (wch >= 0x2A700 && wch <= 0x2B73F) ||
      // 扩展 D 区（CJK Unified Ideographs Extension D）范围：0x2B820 -
      // 0x2CEAF
      (wch >= 0x2B740 && wch <= 0x2B81F) ||
      // 扩展 E 区（CJK Unified Ideographs Extension E）范围：0x2CEB0 -
      // 0x2EBEF
      (wch >= 0x2B820 && wch <= 0x2CEAF) ||
      // 汉字笔画（CJK Compatibility Ideographs）范围：0x2E80 - 0x2EFF
      (wch >= 0xF900 && wch <= 0xFAFF) ||
      // 康熙部首（Kangxi Radicals）范围：0x2F00 - 0x2FDF
      (wch >= 0x2F800 && wch <= 0x2FA1F));
}

}  // namespace strings
}  // namespace kingfisher
