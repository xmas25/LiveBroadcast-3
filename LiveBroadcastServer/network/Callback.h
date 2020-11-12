#ifndef NETWORK_CALLBACK_H
#define NETWORK_CALLBACK_H

#include <vector>
#include <functional>
#include <cstdint>

class Channel;
typedef std::vector<Channel*> ChannelVector;


typedef std::function<void()> EventCallback;
#endif // !NETWORK_CALLBACK_H