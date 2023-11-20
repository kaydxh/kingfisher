
#include "strings/strings.h"

#include <stdarg.h>  //va_start

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>

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

}  // namespace strings
}  // namespace kingfisher
