#include "ft.transfer.h"

#include "log/config.h"
#include "net/http/http_client.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"
#include "strings/strings.h"
#include "time/timer.h"

namespace kingfisher {
namespace file {

FileTransfer::FileTransfer() {}
FileTransfer::~FileTransfer() {}

int FileTransfer::Init(const FileTransferOptions& opts) {
  opts_ = opts;
  return 0;
}

int FileTransfer::Download(std::string& data, const std::string& url) {
  net::HttpClient client;
  client.SetTimeoutMs(opts_.download_timeout_ms);
  net::HttpRequest req;
  req.SetUrl(url);

  net::HttpResponse resp;

  int ret = 0;
  for (int i = 0; i < opts_.retry_times; ++i) {
    ret = client.Get(req, resp);
    if (ret == 0) {
      break;
    }
    LOG(ERROR) << strings::FormatString("failed to download %s, ret: %d",
                                        url.c_str(), ret);
    time::MsSleep(opts_.retry_interval);
  }

  if (ret == 0) {
    data = resp.Body();
  }

  return ret;
}

int FileTransfer::Upload(const std::string& data, const std::string& url) {
  net::HttpClient client;
  client.SetTimeoutMs(opts_.upload_timeout_ms);
  net::HttpRequest req;
  req.SetUrl(url);
  req.SetBody(data);

  net::HttpResponse resp;
  int ret = client.Put(req, resp);
  if (ret != 0) {
    LOG(ERROR) << strings::FormatString(
        "failed to upload len %d to %s, ret: %d", data.size(), url.c_str(),
        ret);

    return ret;
  }

  return 0;
}

}  // namespace file
}  // namespace kingfisher
