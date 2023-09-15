#ifndef KINGFISHER_BASE_PATH_PATH_H_
#define KINGFISHER_BASE_PATH_PATH_H_

#include <string>

namespace kingfisher {
namespace path {

std::string Abs(const std::string &path);

bool IsAbs(const std::string &path);

std::string Dir(const std::string &path);

int Split(const std::string &path, std::string &dir_return_,
          std::string &file_return_);

std::string Join(const std::string &base, const std::string &path);

bool IsPathBeginSeparator(const std::string &path);
bool IsPathEndSeparator(const std::string &path);

// Base returns the last element of path.
std::string Base(const std::string &path);

int LastSlash(std::string &s);

}  // namespace path
}  // namespace kingfisher

#endif
