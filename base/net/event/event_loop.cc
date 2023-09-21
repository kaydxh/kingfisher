#include "event_loop.h"

#include <assert.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <iostream>
#include <memory>

#include "channel.h"
#include "log/config.h"
#include "net/poller/epoll_poller.h"
#include "net/socket/socket.ops.h"
#include "sync/mutex.h"
#include "thread/thread.h"

namespace kingfisher {
namespace net {

const int kPollTimeoutMs = 10000;

int createEventfdOrDie() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG(INFO) << "Failed in eventfd, err: " << evtfd;
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop()
    : poller_(std::make_unique<EPoller>()),
      quit_(false),
      wakeup_fd_(createEventfdOrDie()),
      wakeup_channel_(std::make_unique<Channel>(this, wakeup_fd_)),
      thread_id_(thread::GetTid()) {
  wakeup_channel_->SetReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeup_channel_->EnableReading();
  LOG(INFO) << "init channel: " << wakeup_channel_.get();
}

EventLoop::~EventLoop() {
  LOG(INFO) << "~EventLoop";
  wakeup_channel_->DisableAll();
  wakeup_channel_->Remove();
  if (wakeup_fd_ != -1) {
    ::close(wakeup_fd_);
    wakeup_fd_ = -1;
  }
}

void EventLoop::handleRead() {
  LOG(INFO) << ">>> EventLoop handleRead in" << std::endl;
  uint64_t one = 1;
  ssize_t n = sockets::Read(wakeup_fd_, &one, sizeof(one));
  LOG(INFO) << "EventLoop::handleRead() reads " << n << " bytes";
  if (n != sizeof(one)) {
    LOG(INFO) << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::Wakeup() {
  uint64_t one = 1;
  ssize_t n = sockets::Write(wakeup_fd_, &one, sizeof(one));
  LOG(INFO) << "EventLoop::wakeup() writes " << n << " bytes";
  if (n != sizeof(one)) {
    LOG(INFO) << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::Run() {
  assert(IsInLoopThread());
  quit_ = false;

  while (!quit_) {
    std::vector<Channel*> channels;
    poller_->Poll(channels, kPollTimeoutMs);
    for (auto channel : channels) {
      LOG(INFO) << "channel: " << channel;
      channel->HandleEvent();
    }
    doPendingFunctions();
  }
  quit_ = true;
}

void EventLoop::Quit() {
  LOG(INFO) << ">>> EventLoop::Quit()";
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
    // std::lock_guard<std::mutex> guard(mutex_);
    sync::MutexGuard guard(mutex_);
    pending_functors_.emplace_back(std::move(cb));
  }

  if (!IsInLoopThread()) {
    Wakeup();
  }
}

void EventLoop::AssertInLoopThread() {
  if (!IsInLoopThread()) {
    LOG(FATAL) << "EventLoop was created in threaid:" << thread_id_
               << ", current thread_id:" << thread ::GetTid();
  }
}

void EventLoop::doPendingFunctions() {
  std::vector<Functor> pending_functors;
  {
    sync::MutexGuard guard(mutex_);
    // std::lock_guard<std::mutex> guard(mutex_);
    pending_functors.swap(pending_functors_);
  }

  for (const Functor& f : pending_functors) {
    f();
  }
}

void EventLoop::UpdateChannel(Channel* channel) {
  AssertInLoopThread();
  poller_->AutoUpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  AssertInLoopThread();
  poller_->Delete(channel);
}

//
}  // namespace net
}  // namespace kingfisher
