#include "network/Channel.h"
#include "network/EventLoop.h"
#include "utils/Logger.h"

Channel::Channel(EventLoop* loop, SOCKET sockfd):
	Channel(loop, "default-connection-name", sockfd)
{

}

Channel::Channel(EventLoop* loop, const std::string& connection_name, SOCKET fd) :
    loop_(loop),
	connection_name(connection_name),
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

void Channel::HandleEventWithGuard()
{
	/**
	 * lock增加TcpConnectionPtr的引用计数 防止从TcpServer中erase后 直接销毁TcpConnection
	 *
	 * 否则如果不增加引用计数 当TcpConnection被销毁后, 所管理的Channel也将会被销毁
	 * 在这之后 不能再使用TcpConnection和Channel的任何 成员函数和成员变量
	 *
	 * 有点类似于在一个 new出来的对象的成员函数中 delete自己
	 * 详见 https://stackoverflow.com/questions/7039597/what-will-happen-if-you-do-delete-this-in-a-member-function
	 *
	 * 经过查阅后自己对深度探索C++对象模型 有了更深得理解
	 */

	if (tied_)
	{
		std::shared_ptr<void> guard = tie_.lock();
		if (!guard)
		{
			LOG_WARN("connection: %s is closed", connection_name.c_str());
		}
		else
		{
			HandleEvent();
		}
	}
	else
	{
		HandleEvent();
	}

}

void Channel::SetReadableCallback(const EventCallback& cb)
{
	readable_callback_ = cb;
}

void Channel::SetWritableCallback(const EventCallback& cb)
{
	writable_callback_ = cb;
}

void Channel::EnableReadable()
{
    ep_event_ |= XEPOLLIN;
    Update();
}

void Channel::EnableWritable()
{
	ep_event_ |= XEPOLLOUT;
	Update();
}

void Channel::DisableWritable()
{
	ep_event_ &= ~XEPOLLOUT;
	Update();
}

void Channel::Update()
{
    loop_->Update(this);
}

void Channel::TieConnection(const std::shared_ptr<void>& connection_tie)
{
	tie_ = connection_tie;
	tied_ = true;
}

void Channel::DisableAll()
{
	ep_event_ = 0;
	channel_status_ = CHANNEL_STATUS_DEL;
	Update();
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

	if (event_ & XEPOLLOUT)
	{
		if (writable_callback_)
		{
			writable_callback_();
		}
	}
}

bool Channel::IsWriting() const
{
	return event_ & XEPOLLOUT;
}
