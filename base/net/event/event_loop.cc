#include "event_loop.h"
#include <sys/eventfd.h>
#include "net/socket/socket.ops.h"
#include "channel.h"
#include <iostream>
#include "net/poller/epoll_poller.h"
#include <poll.h>

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

EventLoop::EventLoop()
    : poller_(new EPoller()),
      quit_(false),
      wakeup_fd_(createEventfdOrDie()),
      wakeup_channel_(new Channel(this, wakeup_fd_)) {

  wakeup_channel_->SetReadEvent(std::bind(&EventLoop::handleRead, this));
  // wakeup_channel_->SetReadCallback(std::bind(&EventLoop::handleRead, this));
  // wakeup_channel_->EnableEvent(POLLIN | POLLPRI);
}

EventLoop::~EventLoop() {}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof one) {
    std::cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::Wakeup() {
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof one) {
    std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::Run() {
  quit_ = false;
  std::vector<std::shared_ptr<Channel>> channels;
  while (!quit_) {
    poller_->Poll(channels, 1000);
    for (auto &channel : channels) {
      channel->HandleEvent();
    }
  }
  quit_ = true;
}

//
}
}
