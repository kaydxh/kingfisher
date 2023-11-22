#include "ft.transfer.h"

#include "log/config.h"
#include "net/http/http_client.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"
#include "strings/strings.h"
#include "time/timer.h"
#include "time/wait.h"

namespace kingfisher {
namespace file {

FileTransfer::FileTransfer() {}
FileTransfer::~FileTransfer() {}

int FileTransfer::Init(const FileTransferOptions& opts) {
  opts_ = opts;
  return 0;
}

int FileTransfer::Download(std::string& data, const std::string& url) {
  return time::Retry(
      [&]() -> int {
        net::HttpClient client;
        client.SetTimeoutMs(opts_.download_timeout_ms);
        net::HttpRequest req;
        req.SetUrl(url);

        net::HttpResponse resp;

        int ret = 0;
        ret = client.Get(req, resp);
        if (ret != 0) {
          LOG(ERROR) << strings::FormatString("failed to download %s, ret: %d",
                                              url.c_str(), ret);
          return ret;
        }
        data = resp.Body();
        return 0;
      },
      opts_.retry_interval, opts_.retry_times);
}

int FileTransfer::Upload(const std::string& data, const std::string& url) {
  return time::Retry(
      [&]() -> int {
        net::HttpClient client;
        client.SetTimeoutMs(opts_.upload_timeout_ms);
        net::HttpRequest req;
        req.SetUrl(url);
        req.SetBody(data);

        net::HttpResponse resp;
        int ret = client.Put(req, resp);
        if (ret != 0) {
          LOG(ERROR) << strings::FormatString(
              "failed to upload len %d to %s, ret: %d", data.size(),
              url.c_str(), ret);

          return ret;
        }

        return 0;
      },
      opts_.retry_interval, opts_.retry_times);
}

}  // namespace file
}  // namespace kingfisher
