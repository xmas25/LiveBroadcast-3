#ifndef NETWORK_CALLBACK_H
#define NETWORK_CALLBACK_H

#include <vector>
#include <functional>
#include <cstdint>
#include <memory>

#include "utils/Timestamp.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

class Channel;
class TcpConnection;
class Buffer;


typedef std::vector<Channel*> ChannelVector;


typedef std::function<void()> EventCallback;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr&)> NewConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)> NewMessageCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCloseCallback;
#endif // !NETWORK_CALLBACK_H