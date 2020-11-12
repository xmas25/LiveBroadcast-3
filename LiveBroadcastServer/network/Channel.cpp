#include "network/Channel.h"
#include "network/EventLoop.h"

enum EPOLL_EVENTS
{
    EPOLLIN = 0x001,
#define EPOLLIN EPOLLIN
    EPOLLPRI = 0x002,
#define EPOLLPRI EPOLLPRI
    EPOLLOUT = 0x004,
#define EPOLLOUT EPOLLOUT
    EPOLLRDNORM = 0x040,
#define EPOLLRDNORM EPOLLRDNORM
    EPOLLRDBAND = 0x080,
#define EPOLLRDBAND EPOLLRDBAND
    EPOLLWRNORM = 0x100,
#define EPOLLWRNORM EPOLLWRNORM
    EPOLLWRBAND = 0x200,
#define EPOLLWRBAND EPOLLWRBAND
    EPOLLMSG = 0x400,
#define EPOLLMSG EPOLLMSG
    EPOLLERR = 0x008,
#define EPOLLERR EPOLLERR
    EPOLLHUP = 0x010,
#define EPOLLHUP EPOLLHUP
    EPOLLRDHUP = 0x2000,
#define EPOLLRDHUP EPOLLRDHUP
    EPOLLWAKEUP = 1u << 29,
#define EPOLLWAKEUP EPOLLWAKEUP
    EPOLLONESHOT = 1u << 30,
#define EPOLLONESHOT EPOLLONESHOT
    EPOLLET = 1u << 31
#define EPOLLET EPOLLET
};

Channel::Channel(EventLoop* loop, int fd) :
    loop_(loop),
	fd_(fd),
	channel_status_(CHANNEL_STATUS_NEW),
	event_(0),
	ep_event_(0)
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

int Channel::GetSockFd() const
{
	return fd_;
}

void Channel::HandleEvent()
{
	if (event_ & EPOLLIN)
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
    ep_event_ |= EPOLLIN;
    Update();
}

void Channel::Update()
{
    loop_->Update(this);
}
