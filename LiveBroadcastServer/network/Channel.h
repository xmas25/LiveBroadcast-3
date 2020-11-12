#ifndef NETWORK_CHANNEL
#define NETWORK_CHANNEL

#include "network/Callback.h"

class EventLoop;
class Channel
{
public:
	enum ChannelStatus
	{
		CHANNEL_STATUS_NEW,
		CHANNEL_STATUS_ADDED,
		CHANNEL_STATUS_DEL
	};

	Channel(EventLoop* loop, int sockfd);

	void SetEvent(uint32_t ev);
	uint32_t GetEpollEvent() const;

	ChannelStatus GetChannelStatus() const;
	void SetChannelStatus(ChannelStatus channel_status);

	int GetSockFd() const;

	void HandleEvent();

	void SetReadableCallback(const EventCallback& cb);

	void EnableReadable();

private:
	EventLoop* loop_;

	int fd_;
	ChannelStatus channel_status_;
	uint32_t event_;
	uint32_t ep_event_;

	EventCallback readable_callback_;

	void Update();
};

#endif