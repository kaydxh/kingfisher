#include "event_loop.h"

#include <assert.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <iostream>
#include <memory>

#include "channel.h"
#include "net/poller/epoll_poller.h"
#include "net/socket/socket.ops.h"
#include "thread/thread.h"

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
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      thread_id_(thread::GetTid()) {
  wakeup_channel_->SetReadEvent(std::bind(&EventLoop::handleRead, this));
  poller_->Add(wakeup_channel_, 0);
  std::cout << "init channel: " << wakeup_channel_.get() << std::endl;
}

EventLoop::~EventLoop() {
  std::cout << "~EventLoop" << std::endl;
  if (wakeup_fd_ != -1) {
    ::close(wakeup_fd_);
    wakeup_fd_ = -1;
  }
}

void EventLoop::handleRead() {
  std::cout << "handleRead in" << std::endl;
  uint64_t one = 1;
  ssize_t n = sockets::Read(wakeup_fd_, &one, sizeof(one));
  std::cout << "EventLoop::handleRead() reads " << n << " bytes" << std::endl;
  if (n != sizeof one) {
    std::cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8"
              << std::endl;
  }
}

void EventLoop::Wakeup() {
  uint64_t one = 1;
  ssize_t n = sockets::Write(wakeup_fd_, &one, sizeof(one));
  std::cout << "EventLoop::wakeup() writes " << n << " bytes" << std::endl;
  if (n != sizeof(one)) {
    std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8"
              << std::endl;
  }
}

void EventLoop::Run() {
  assert(IsInLoopThread());
  quit_ = false;
  std::vector<std::shared_ptr<Channel>> channels;
  while (!quit_) {
    poller_->Poll(channels, 1000);
    for (auto channel : channels) {
      std::cout << "channel: " << channel.get() << std::endl;
      channel->HandleEvent();
    }
  }
  quit_ = true;
}

void EventLoop::Quit() {
  quit_ = true;
  if (!IsInLoopThread()) {
    Wakeup();
  }
}

bool EventLoop::IsInLoopThread() const {
  return thread_id_ == thread::GetTid();
}

void EventLoop::RunInLoop(Functor&& cb) {
  if (IsInLoopThread()) {
    cb();
    return;
  }

  QueueInLoop(std::move(cb));
}

void EventLoop::QueueInLoop(Functor&& cb) {
  {
    std::lock_guard<std::mutex> guard(mutex_);
    pending_functors_.emplace_back(std::move(cb));
  }

  if (!IsInLoopThread()) {
    Wakeup();
  }
}
//
}  // namespace net
}  // namespace kingfisher
