#include "epoll_poller.h"
#include <sys/epoll.h>
#include <assert.h>
#include <unistd.h>
#include <cstring>
#include "net/event/channel.h"
#include <iostream>
//#include "timestamp.h"

namespace kingfisher {
namespace net {

// EPOLL_CLOEXEC Set the close-on-exec (FD_CLOEXEC) flag on the new file
// descriptor. See the description of the O_CLOEXEC flag in open(2) for reasons
// why this may be useful
EPoller::EPoller(int maxevents)
    : epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)), events_(maxevents) {
  assert(epoll_fd_ >= 0);
}

EPoller::~EPoller() {
  if (epoll_fd_ >= 0) {
    ::close(epoll_fd_);
    epoll_fd_ = -1;
  }
}
int EPoller::Poll(std::vector<std::shared_ptr<Channel>>& channels_reutrn,
                  int timeout_ms) {
  int events_cout = ::epoll_wait(epoll_fd_, &*events_.begin(),
                                 static_cast<int>(events_.size()), timeout_ms);
  std::cout << "epoll fd: " << epoll_fd_ << " events cout:" << events_cout
            << std::endl;
  if (events_cout < 0) {
    std::cout << errno << std::endl;
  }
  // time::Timestamp::Now();
  for (int i = 0; i < events_cout; ++i) {
    std::shared_ptr<Channel> channel =
        *static_cast<std::shared_ptr<Channel>*>(events_[i].data.ptr);
    // auto fd = events_[i].data.fd;
    channel->SetRevents(events_[i].events);
    channels_reutrn.push_back(channel);
  }

  return 0;
}

int EPoller::Add(std::shared_ptr<Channel> channel, int timeout_ms) { return 0; }

int EPoller::Update(std::shared_ptr<Channel> channel, int timeout_ms) {
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = channel->Events();
  event.data.fd = channel->Fd();
  event.data.ptr = &channel;

  // fd target file descriptor
  int ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, channel->Fd(), &event);
  if (ret != 0) {
    return ret;
  }

  return 0;
}
int EPoller::Delete(std::shared_ptr<Channel> channel, int timeout_ms) {
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = channel->Events();
  event.data.fd = channel->Fd();
  //  event.data.ptr = channel;

  // fd target file descriptor
  int ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, channel->Fd(), &event);
  if (ret != 0) {
    return ret;
  }

  return 0;
}
}
}
