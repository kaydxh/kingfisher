#ifndef KINGFISHER_BASE_NET_POLL_POLLER_H_
#define KINGFISHER_BASE_NET_POLL_POLLER_H_

#include <map>
#include <memory>
#include <vector>

namespace kingfisher {
namespace net {

class Channel;

class IPoller {
 public:
  IPoller();
  virtual ~IPoller();

  virtual int Poll(std::vector<Channel*>& channels, int timeout_ms) = 0;
  virtual int Add(Channel* channel) = 0;
  virtual int Update(Channel* channel) = 0;
  virtual int Delete(Channel* channel) = 0;
  virtual void AutoUpdateChannel(Channel* channel) = 0;

 protected:
  std::map<int, Channel*> fd_channels_;
};
}  // namespace net
}  // namespace kingfisher

#endif
