
#ifndef KINGFISHER_BASE_STRINGS_STRINGS_H_
#define KINGFISHER_BASE_STRINGS_STRINGS_H_

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

namespace kingfisher {
namespace strings {

bool IsNumber(const std::string &s);

int ParseInt64(int64_t &result, const std::string &s, int base = 10);

int ParseUint64(uint64_t &result, const std::string &s, int base = 10);

std::vector<std::string> Split(const std::string &s, const char delim);
std::vector<int64_t> SplitToInt64(const std::string &s, const char delim);
std::string Join(const std::vector<std::string> &elems, const std::string &sep);

template <typename T>
std::string Join(const std::vector<T> &elems, const std::string &sep) {
  int sz = elems.size();
  if (sz == 0) {
    return "";
  }
  std::stringstream ss;
  ss << elems[0];
  if (sz == 1) {
    return ss.str();
  }

  for (auto i = 1; i < sz; ++i) {
    ss << sep;
    ss << elems[i];
  }

  return ss.str();
}

template <typename T>
T StringToType(const std::string &str) {
  std::istringstream ss(str);
  T value;
  ss >> value;
  return value;
}

bool HasPrefix(const std::string &s, const std::string &prefix,
               bool case_sensitive = true);
bool HasSuffix(const std::string &s, const std::string &suffix,
               bool case_sensitive = true);

std::string ToLower(const std::string &s);
std::string TrimLeft(const std::string &s, const std::string &prefix);
std::string TrimRight(const std::string &s, const std::string &suffix);
std::string Trim(const std::string &s);

bool Contains(const std::string &s, const std::string &sub);

bool IsOctalDigit(char c);
//{0-9} or {A-F}
bool IsHexDigit(unsigned char c);
unsigned int HexDigitToInt(char c);

bool IsSurrogate(char32_t c, std::string src);

bool Unescape(std::string &dest, const std::string &source,
              bool leave_nulls_escaped);

std::string FormatString(const char *fmt, ...);

template <typename T>
T ToNumber(std::string s) {
  T t;
  const char *data = s.data();
  std::memcpy(&t, data, sizeof(T));

  return t;
}

template <typename T>
std::vector<T> ToNumbers(std::string s) {
  std::vector<T> results;
  const char *data = s.data();
  int size = s.size();

  for (int i = 0; i < size; i += sizeof(T)) {
    T t;
    std::memcpy(&t, data + i, sizeof(T));
    results.push_back(t);
  }

  return results;
}

int WStringFromBytes(std::wstring &result, const std::string &s);
int WStringToBytes(std::string &result, const std::wstring &ws);
bool IsChineseChar(const wchar_t wch);

}  // namespace strings
}  // namespace kingfisher
#endif
