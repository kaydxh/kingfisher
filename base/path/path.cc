#include "path/path.h"

#include <regex.h>

namespace kingfisher {
namespace path {

std::string Abs(const std::string &path) {
  char abs_path[256] = {0};
  if (realpath(path.c_str(), abs_path)) {
    return std::string(abs_path);
  }

  return "";
}

bool IsAbs(const std::string &path) {
  return path.length() > 0 && path[0] == '/';
}

std::string Dir(const std::string &path) {
  std::string dir;
  std::string filename;
  auto ret = Split(path, dir, filename);
  if (ret != 0) {
    return "";
  }
  return dir;
}

int Split(const std::string &path, std::string &dir_return_,
          std::string &file_return_) {
  auto pos = path.find_last_of("/");
  if (pos != std::string::npos) {
    dir_return_ = path.substr(0, pos + 1);
    file_return_ = path.substr(pos + 1);
    return 0;
  }

  return -1;
}

bool IsPathBeginSeparator(const std::string &path) {
  if (path.empty()) {
    return false;
  }

  char c = path[0];
  return c == '/' || c == '\\';
}

bool IsPathEndSeparator(const std::string &path) {
  if (path.empty()) {
    return false;
  }

  char c = path[path.size() - 1];
  return c == '/' || c == '\\';
}

std::string Join(const std::string &base, const std::string &path) {
  if (base.empty()) {
    return path;
  }
  if (path.empty()) {
    return base;
  }

  std::string result;
  bool base_path_end_sep = IsPathEndSeparator(base);
  bool path_begin_sep = IsPathBeginSeparator(path);

  if (base_path_end_sep && path_begin_sep) {
    result = base + path.substr(1);
  } else if (!base_path_end_sep && !path_begin_sep) {
    result = base + "/" + path;
  } else {
    result = base + path;
  }

  return result;
}

int LastSlash(std::string &s) {
  int i = s.size() - 1;
  while (i >= 0 && s[i] != '/') {
    i--;
  }
  return i;
}

std::string Base(const std::string &path) {
  if (path.empty()) {
    return ".";
  }

  std::string temp_path = path;
  for (; temp_path.size() > 0 && temp_path[temp_path.size() - 1] == '/';) {
    temp_path = temp_path.substr(0, temp_path.size() - 1);
  }
  int i = LastSlash(temp_path);
  if (i >= 0) {
    temp_path = temp_path.substr(i + 1);
  }
  if (temp_path.empty()) {
    return "/";
  }
  return temp_path;
}

}  // namespace path
}  // namespace kingfisher
