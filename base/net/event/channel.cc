#include "channel.h"

#include <poll.h>

#include <iostream>
#include <memory>
#include <sstream>

#include "log/config.h"
#include "net/event/event_loop.h"

namespace kingfisher {
namespace net {

Channel::Channel(EventLoop* loop, int fd) : loop_(loop), fd_(fd) {}

Channel::~Channel() { std::cout << "~Channel" << std::endl; }

void Channel::SetReadCallback(EventCallback cb) {
  readCallback_ = std::move(cb);
}

void Channel::SetWriteCallback(EventCallback cb) {
  writeCallback_ = std::move(cb);
}

void Channel::SetCloseCallback(EventCallback cb) {
  closeCallback_ = std::move(cb);
}

void Channel::EnableEvent(int event) {
  events_ |= event;
  Update();
}

void Channel::DisableEvent(int event) {
  events_ &= ~event;
  Update();
}

void Channel::EnableReading() { EnableEvent(POLLIN | POLLPRI); }

void Channel::SetRevents(int revents) { revents_ = revents; }

void Channel::EnableWriting() { EnableEvent(POLLOUT); }

int Channel::Events() const { return events_; }

void Channel::DisableReading() { DisableEvent(POLLIN); }

void Channel::DisableWriting() { DisableEvent(POLLOUT); }

void Channel::DisableAll() {
  events_ = kNoneEvent;
  Update();
}

int Channel::Fd() const { return fd_; }

void Channel::Update() { loop_->UpdateChannel(this); }

void Channel::HandleEvent() {
  LOG(INFO) << ">>> HandleEvent: " << reventsToString() << std::endl;
  // close event
  // POLLHUP: Hang up (output only)
  // POLLIN: There is data to read
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (closeCallback_) {
      closeCallback_();
    }
  }

  // error event
  // POLLNVAL: Invalid request: fd not open (output only)
  // POLLERR: Error condition (output only).
  if (revents_ & (POLLERR | POLLNVAL)) {
  }

  // read event
  // POLLPRI: There is urgent data to read (e.g., out-of-band data on TCP
  // socket; pseudoterminal master in packet mode has seen state change in
  // slave).
  // POLLRDHUP: (since Linux 2.6.17) Stream socket peer closed connection, or
  // shut down writing half of connection. The _GNU_SOURCE feature test macro
  // must be defined (before including any header files) in order to obtain this
  // definition.
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readCallback_) {
      readCallback_();
    }
  }

  // write event
  if (revents_ & POLLOUT) {
    if (writeCallback_) {
      writeCallback_();
    }
  }
}

std::string Channel::reventsToString() {
  //  std::ostringstream result;
  char buf[1024] = {0};
  snprintf(buf, sizeof(buf), "fd[%d]:events[ %s]", fd_,
           eventsToString(revents_).data());
  return std::string(buf);
}

std::string Channel::eventsToString(int events) {
  std::ostringstream result;
  if (events & POLLIN) {
    result << "POLLIN ";
  }
  if (events & POLLPRI) {
    result << "POLLPRI ";
  }
  if (events & POLLOUT) {
    result << "POLLOUT ";
  }
  if (events & POLLHUP) {
    result << "POLLHUP ";
  }
  if (events & POLLRDHUP) {
    result << "POLLRDHUP ";
  }
  if (events & POLLERR) {
    result << "POLLERR ";
  }
  if (events & POLLNVAL) {
    result << "POLLNVAL ";
  }
  return result.str();
}
void Channel::Remove() { loop_->RemoveChannel(this); }

}  // namespace net
}  // namespace kingfisher
