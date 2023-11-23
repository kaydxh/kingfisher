
#ifndef KINGFISHER_BASE_STRINGS_STRINGS_H_
#define KINGFISHER_BASE_STRINGS_STRINGS_H_

#include <cstring>
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

bool HasPrefix(const std::string &s, const std::string &prefix,
               bool case_sensitive = true);
bool HasSuffix(const std::string &s, const std::string &suffix,
               bool case_sensitive = true);

std::string ToLower(const std::string &s);
std::string TrimLeft(const std::string &s, const std::string &prefix);
std::string TrimRight(const std::string &s, const std::string &suffix);
std::string Trim(const std::string &s);

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

}  // namespace strings
}  // namespace kingfisher
#endif
