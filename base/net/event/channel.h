#ifndef KINGFISHER_BASE_NET_EVENT_CHANNEL_H_
#define KINGFISHER_BASE_NET_EVENT_CHANNEL_H_

#include <functional>
#include <memory>

#include "time/timestamp.h"

namespace kingfisher {
namespace net {

class EventLoop;

const int kNoneEvent = 0;

enum EventStatus {
  kEventNew = -1,
  kEventAdded = 1,
  kEventDeleted = 2,
};

// Channel的生命周期由TcpConnection管理
class Channel {
 public:
  using EventCallback = std::function<void()>;
  // using ReadEventCallback = std::function<void(time::Timestamp)>;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void SetReadCallback(EventCallback cb);
  void SetWriteCallback(EventCallback cb);
  void SetCloseCallback(EventCallback cb);
  void SetRevents(int revents);

  void EnableEvent(int event);
  void EnableReading();
  void EnableWriting();

  void DisableEvent(int event);
  void DisableReading();
  void DisableWriting();
  void DisableAll();

  void Tie(const std::shared_ptr<void>&);

  EventStatus GetEventStatus() const { return event_status_; }
  void SetEventStatus(EventStatus es) { event_status_ = es; }

  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  // void Tie(const std::shared_ptr<void>&);
  int Events() const;
  int Fd() const;
  void HandleEvent();
  void Remove();
  void Update();
  void HandleEventWithGuard();

 private:
  void update();
  std::string reventsToString();
  std::string eventsToString(int events);

  EventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;

  EventLoop* loop_ = nullptr;
  EventStatus event_status_ = kEventNew;
  const int fd_;
  int events_ = 0;
  // received event type of epoller
  int revents_ = 0;

  std::weak_ptr<void> tie_;
  bool tied_ = false;
};
}  // namespace net
}  // namespace kingfisher

#endif
