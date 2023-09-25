#include <gtest/gtest.h>

#include "core/noncopyable.hpp"
#include "net/event/callback.h"
#include "net/event/channel.h"
#include "net/event/event_loop.h"
#include "net/socket/net.buffer.h"
#include "net/socket/socket_addr.h"
#include "net/tcp/tcp_client.h"
#include "net/tcp/tcp_server.h"

using namespace kingfisher;
using namespace kingfisher::net;
using namespace kingfisher::thread;

class test_TcpClient : public testing::Test {
 public:
  test_TcpClient() {}
  ~test_TcpClient() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

class EchoClient : public kingfisher::noncopyable_::noncopyable {
 public:
  EchoClient(EventLoop* loop, const sockets::SockAddress& listen_addr)
      : loop_(loop), client_(loop, listen_addr) {
    client_.SetConnectionCallback(
        std::bind(&EchoClient::onConnection, this, std::placeholders::_1));
    client_.SetMessageCallback(std::bind(&EchoClient::onMessage, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
    std::cout << "=========EchoClient=======" << std::endl;
  }

  void Connect() { client_.Connect(); }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    std::cout << "=========onConnection =======" << std::endl;
    conn->Send("word\n");
  }

  void onMessage(const TcpConnectionPtr& conn, NetBuffer* buf) {
    std::cout << "--onMessage--" << std::endl;
  }

 private:
  EventLoop* loop_;
  TcpClient client_;
};

TEST_F(test_TcpClient, All) {
  EventLoop loop;
  sockets::SockAddress listen_addr(10000);
  EchoClient client(&loop, listen_addr);
  client.Connect();
  loop.Run();
}
