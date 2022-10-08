#include "event_loop.h"
#include <sys/eventfd.h>
#include "net/socket/socket.ops.h"
#include "channel.h"
#include <iostream>

namespace kingfisher {
namespace net {

int createEventfdOrDie() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    std::cout << "Failed in eventfd, err: " << evtfd << std::endl;
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop() : quit_(false), wakeupFd_(createEventfdOrDie()) {

  wakeup_channel_->SetReadEvent(std::bind(&EventLoop::handleRead, this));
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof(one));
  if (n != sizeof one) {
    std::cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::Run() {}
}
}
