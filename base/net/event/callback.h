#ifndef KINGFISHER_BASE_NET_EVENT_CALLBACK_H_
#define KINGFISHER_BASE_NET_EVENT_CALLBACK_H_

#include <functional>

#include "time/timestamp.h"

namespace kingfisher {
namespace net {

class NetBuffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback =
    std::function<void(const TcpConnectionPtr&, NetBuffer*)>;

}  // namespace net
}  // namespace kingfisher

#endif
