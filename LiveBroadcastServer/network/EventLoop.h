#ifndef NETWORK_EVENTLOOP_H
#define NETWORK_EVENTLOOP_H

#include "network/Callback.h"

class MultiplexingBase;
class Channel;
class EventLoop
{
public:

	EventLoop();
	~EventLoop();

	void Loop();

	void Update(Channel* channel);

private:

	MultiplexingBase* multiplexing_base_;

	ChannelVector active_channels_;

	bool looping_;

	void HandleActiveChannel();
};

#endif // NETWORK_EVENTLOOP_H