#ifndef KINGFISHER_BASE_NET_EVENT_CHANNEL_H_
#define KINGFISHER_BASE_NET_EVENT_CHANNEL_H_

#include <functional>
#include <memory>
#include "time/timestamp.h"

namespace kingfisher {
namespace net {

class EventLoop;

class Channel {
 public:
  using EventCallback = std::function<void()>;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void SetReadCallback(EventCallback cb);
  void SetWriteCallback(EventCallback cb);
  void SetRevents(int revents);

  void SetReadEvent(EventCallback cb);

  void EnableEvent(int event);

  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void Tie(const std::shared_ptr<void>&);
  int Events() const;
  int Fd() const;
  void HandleEvent();

 private:
  void update();
  std::string reventsToString();
  std::string eventsToString(int events);

  EventCallback readCallback_;
  EventCallback writeCallback_;

  EventLoop* loop_ = nullptr;
  const int fd_;
  int events_;
  // received event type of epoller
  int revents_;
  std::weak_ptr<void> tie_;
  bool tied_ = false;
};
}
}

#endif
