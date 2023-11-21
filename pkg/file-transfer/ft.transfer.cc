#include "ft.transfer.h"

#include "log/config.h"
#include "net/http/http_client.h"
#include "net/http/http_request.h"
#include "net/http/http_response.h"
#include "strings/strings.h"

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
  int ret = client.Get(req, resp);
  if (ret != 0) {
    LOG(ERROR) << strings::FormatString("failed to download %s, ret: %d", url,
                                        ret);

    return ret;
  }
  data = resp.Body();

  return 0;
}

}  // namespace file
}  // namespace kingfisher
