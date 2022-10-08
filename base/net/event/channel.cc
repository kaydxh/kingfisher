#include "channel.h"
#include <poll.h>
#include <sstream>

namespace kingfisher {
namespace net {

Channel::Channel(EventLoop* loop, int fd) : loop_(loop), fd_(fd) {}

void Channel::SetReadCallback(ReadEventCallback cb) {
  readCallback_ = std::move(cb);
}

void Channel::SetWriteCallback(EventCallback cb) {
  writeCallback_ = std::move(cb);
}

void Channel::SetReadEvent(ReadEventCallback cb) {
  SetReadCallback(cb);
  EnableEvent(POLLIN | POLLPRI);
}

void Channel::EnableEvent(int event) { events_ |= event; }

void Channel::SetRevents(int revents) { revents_ = revents; }

void Channel::Tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

int Channel::Events() const { return events_; }

int Channel::Fd() const { return fd_; }

void Channel::update() {
  // loop_->
}

void Channel::handleEvent(time::Timestamp receivedTime) {
  std::shared_ptr<void> guard;
  if (tied_) {
    guard = tie_.lock();
    if (guard) {
      return doHandleEvent(receivedTime);
    }
  }
  return doHandleEvent(receivedTime);
}

void Channel::doHandleEvent(time::Timestamp receivedTime) {
  // close event
  // POLLHUP: Hang up (output only)
  // POLLIN: There is data to read
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
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
      readCallback_(receivedTime);
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
  snprintf(buf, sizeof(buf), "fd[%d]:events[%s]", fd_,
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
}
}
