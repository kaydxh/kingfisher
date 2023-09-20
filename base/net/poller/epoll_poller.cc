#include "epoll_poller.h"

#include <assert.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <memory>

#include "net/event/channel.h"
// #include "timestamp.h"

namespace kingfisher {
namespace net {

// EPOLL_CLOEXEC Set the close-on-exec (FD_CLOEXEC) flag on the new file
// descriptor. See the description of the O_CLOEXEC flag in open(2) for reasons
// why this may be useful
EPoller::EPoller(int maxevents)
    : epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)), events_(maxevents) {
  assert(Validate());
}

EPoller::~EPoller() {
  std::cout << "~EPoller()" << std::endl;
  if (Validate()) {
    ::close(epoll_fd_);
    epoll_fd_ = -1;
  }
}

bool EPoller::Validate() const { return epoll_fd_ >= 0; }

int EPoller::Poll(std::vector<Channel*>& channels_reutrn, int timeout_ms) {
  int events_cout = ::epoll_wait(epoll_fd_, &*events_.begin(),
                                 static_cast<int>(events_.size()), timeout_ms);
  std::cout << "epoll fd: " << epoll_fd_ << " events cout:" << events_cout
            << std::endl;
  if (events_cout < 0) {
    std::cout << errno << std::endl;
  }

  // time::Timestamp::Now();
  for (int i = 0; i < events_cout; ++i) {
#if 0
    std::shared_ptr<Channel> channel =
        *reinterpret_cast<std::shared_ptr<Channel>*>(events_[i].data.ptr);
#endif
    Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
    channel->SetRevents(events_[i].events);
    channels_reutrn.push_back(channel);
  }

  return 0;
}

int EPoller::Add(Channel* channel) { return operate(EPOLL_CTL_ADD, channel); }

int EPoller::Update(Channel* channel) {
  return operate(EPOLL_CTL_MOD, channel);
}

int EPoller::Delete(Channel* channel) {
  return operate(EPOLL_CTL_DEL, channel);
}

int EPoller::operate(int operation, Channel* channel) {
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = channel->Events();
  event.data.fd = channel->Fd();
  // void = shared_ptr, note: need new shared_ptr
  // https://stackoverflow.com/questions/31063055/passing-a-shared-ptr-through-a-c-interface-that-takes-void#:~:text=evt.user.data1%20%3D%20new%20MessagePtr%20(msg)%3B
  // event.data.ptr = new std::shared_ptr<Channel>(channel);
  event.data.ptr = channel;

  // fd target file descriptor
  int ret = ::epoll_ctl(epoll_fd_, operation, channel->Fd(), &event);
  if (ret != 0) {
    return ret;
  }

  return 0;
}
}  // namespace net
}  // namespace kingfisher
