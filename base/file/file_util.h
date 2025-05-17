//
// Created by kayxhding on 2020-06-05 11:03:36
//

#ifndef KINGFISHER_BASE_FILE_FILE_UTIL_H_
#define KINGFISHER_BASE_FILE_FILE_UTIL_H_

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <cerrno>
#include <iostream>

namespace kingfisher {
namespace fileutil {
namespace detail {

// Wrap call to f (args) in loop to retry on EINTR
template <typename Func, typename... Args>
ssize_t WrapFuncT(Func f, Args... args) {
  ssize_t ret = 0;
  do {
    ret = f(args...);
  } while (-1 == ret && EINTR == errno);

  return ret;
}

inline void Incr(ssize_t) {}
inline void Incr(ssize_t n, off_t &offset) { offset += n; }

template <typename Func, typename... Offset>
ssize_t WrapFileOpFuncT(Func f, int fd, void *buf, size_t size,
                        Offset... offset) {
  ssize_t ret = -1;
  ssize_t total_bytes = 0;
  char *b = static_cast<char *>(buf);
  do {
    ret = f(fd, b, size, offset...);
    if (-1 == ret) {
      if (EINTR == errno || EAGAIN == errno) {
        continue;
      }

      return ret;
    }

    total_bytes += ret;
    b += ret;
    size -= ret;
    Incr(ret, offset...);
  } while (0 != ret && size);

  return total_bytes;
}

}  // namespace detail

int Open(const char *filename, int flags = O_RDWR | O_LARGEFILE | O_CREAT,
         mode_t mode = 0666);

ssize_t ReadFull(int fd, void *buf, size_t length);

ssize_t WriteFull(int fd, const void *buf, size_t length);

ssize_t WriteFile(const std::string &filename, const std::string &content,
                  bool apppended);

bool CopyFile(const char *from_path, const char *to_path);

std::string Dir(const std::string &filename);

int MakeDirAll(const std::string &dir);

bool IsDir(const std::string &path);

int ListFiles(const std::string& dir, std::vector<std::string>& files);

bool DeleteFile(const char *filename);

int Flock(int fd, int operation);

}  // namespace fileutil
}  // namespace kingfisher

#endif
