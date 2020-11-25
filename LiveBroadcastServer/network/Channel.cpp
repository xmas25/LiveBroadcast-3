#include "network/Channel.h"
#include "network/EventLoop.h"

Channel::Channel(EventLoop* loop, SOCKET fd) :
    loop_(loop),
	fd_(fd),
	channel_status_(CHANNEL_STATUS_NEW),
	event_(0),
	ep_event_(0),
	tied_(false)
{

}

void Channel::SetEvent(uint32_t ev)
{
	event_ = ev;
}

uint32_t Channel::GetEpollEvent() const
{
	return ep_event_;
}

Channel::ChannelStatus Channel::GetChannelStatus() const
{
	return channel_status_;
}

void Channel::SetChannelStatus(ChannelStatus channel_status)
{
	channel_status_ = channel_status;
}

SOCKET Channel::GetSockFd() const
{
	return fd_;
}

void Channel::HandleEvent()
{
	if (event_ & XEPOLLIN)
	{
        if (readable_callback_)
        {
            readable_callback_();
        }
	}
}

void Channel::SetReadableCallback(const EventCallback& cb)
{
	readable_callback_ = cb;
}

void Channel::EnableReadable()
{
    ep_event_ |= XEPOLLIN;
    Update();
}

void Channel::Update()
{
    loop_->Update(this);
}

void Channel::TieConnection(const std::shared_ptr<void>& connection_tie)
{
	connection_tie_ = connection_tie;
	tied_ = true;
}

void Channel::DisableAll()
{
	ep_event_ = 0;
	Update();
}
