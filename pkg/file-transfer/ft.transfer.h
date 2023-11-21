#ifndef KINGFISHER_PKG_FILE_TRANSFER_FT_TRANSFER_H_
#define KINGFISHER_PKG_FILE_TRANSFER_FT_TRANSFER_H_

#include <string>

namespace kingfisher {
namespace file {

struct FileTransferOptions {
  int64_t download_timeout_ms;
  int64_t upload_timeout_ms;
  int retry_times;
  int64_t retry_interval;
};

class FileTransfer {
 public:
  FileTransfer();
  ~FileTransfer();

  int Init(const FileTransferOptions& opts);

  int Download(std::string& data, const std::string& url);

 private:
  FileTransferOptions opts_;
};

}  // namespace file
}  // namespace kingfisher

#endif
