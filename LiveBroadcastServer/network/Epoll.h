#ifndef NETWORK_EPOLL_H
#define NETWORK_EPOLL_H

#include <sys/epoll.h>

#include "network/Callback.h"

class Epoll
{
public:
	typedef std::vector<epoll_event> EventVector;
	Epoll();

	bool LoopOnce(int timeout, ChannelVector* active_channels);

	void UpdateChannel(Channel *channel);

private:
	int epfd_;

	EventVector event_vector_;

	void FillActiveSocketVector(int nums, ChannelVector* active_channels);

	void EpollControl(int ctl, int fd, void* ptr, uint32_t ev);

	void RemoveChannel(Channel* channel);

	void UpdateChannelInternal(int ctl, Channel* channel);
};

#endif // !NETWORK_EPOLL_H
