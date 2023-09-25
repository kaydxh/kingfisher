#include <gtest/gtest.h>

#include "net/event/callback.h"
#include "net/event/channel.h"
#include "net/socket/net.buffer.h"
#include "net/socket/socket_addr.h"
#include "net/tcp/tcp_server.h"

using namespace kingfisher;
using namespace kingfisher::net;
using namespace kingfisher::thread;

class test_TcpServer : public testing::Test {
 public:
  test_TcpServer() {}
  ~test_TcpServer() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

class EchoServer {
 public:
  EchoServer(EventLoop* loop, const sockets::SockAddress& listen_addr)
      : loop_(loop), server_(loop, listen_addr, 1) {
    server_.SetConnectionCallback(
        std::bind(&EchoServer::onConnection, this, std::placeholders::_1));

    server_.SetMessageCallback(std::bind(&EchoServer::onMessage, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
  }

  void Start() { server_.Start(); }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    std::cout << "--onConnection--" << conn->Name() << std::endl;
    conn->Send("Hello word\n");
  }

  void onMessage(const TcpConnectionPtr& conn, NetBuffer* buf) {
    std::cout << "--onMessage--" << std::endl;
  }

 private:
  EventLoop* loop_;
  TcpServer server_;
};

TEST_F(test_TcpServer, All) {
  EventLoop loop;
  sockets::SockAddress listen_addr(10000);
  EchoServer server(&loop, listen_addr);
  server.Start();
  loop.Run();
}
