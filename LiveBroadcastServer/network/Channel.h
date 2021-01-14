#ifndef NETWORK_CHANNEL
#define NETWORK_CHANNEL

#include "network/PlatformNetwork.h"
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

	Channel(EventLoop* loop, SOCKET sockfd);
	Channel(EventLoop* loop, const std::string& connection_name, SOCKET sockfd);

	void SetEvent(uint32_t ev);
	uint32_t GetEpollEvent() const;

	ChannelStatus GetChannelStatus() const;
	void SetChannelStatus(ChannelStatus channel_status);

	SOCKET GetSockFd() const;

	void HandleEventWithGuard();

	void SetReadableCallback(const EventCallback& cb);

	void SetWritableCallback(const EventCallback& cb);

	void EnableReadable();

	void EnableWritable();

	void DisableWritable();

	void DisableAll();

	void TieConnection(const std::shared_ptr<void>& connection_tie);

	bool IsWriting() const;
private:
	EventLoop* loop_;

	std::string connection_name;

	SOCKET fd_;
	ChannelStatus channel_status_;
	/* 多路复用返回的事件*/
	uint32_t event_;
	/* 用于注册的事件*/
	uint32_t ep_event_;

	EventCallback readable_callback_;
	EventCallback writable_callback_;

	std::weak_ptr<void> tie_;
	bool tied_;

	void Update();

	void HandleEvent();
};

#endif