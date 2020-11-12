#ifndef NETWORK_EVENTLOOP_H
#define NETWORK_EVENTLOOP_H

#include "network/Callback.h"

class Epoll;
class Channel;
class EventLoop
{
public:

	EventLoop();
	~EventLoop();

	void Loop();

	void Update(Channel* channel);

private:

	Epoll* epoll_;

	ChannelVector active_channels_;

	bool looping_;

	void HandleActiveChannel();
};

#endif // NETWORK_EVENTLOOP_H