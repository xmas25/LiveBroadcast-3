#include "network/EventLoop.h"
#include "network/Channel.h"
#include "network/Epoll.h"

EventLoop::EventLoop() :
	epoll_(new Epoll()),
	active_channels_(),
	looping_(false)
{

}

EventLoop::~EventLoop()
{
	delete epoll_;
}

void EventLoop::Loop()
{
	looping_ = true;

	while (looping_)
	{
		active_channels_.clear();
		looping_ = epoll_->LoopOnce(20, &active_channels_);

		HandleActiveChannel();
	}
}

void EventLoop::Update(Channel* channel)
{
	epoll_->UpdateChannel(channel);
}

void EventLoop::HandleActiveChannel()
{

	for (Channel* channel : active_channels_)
	{
		channel->HandleEvent();
	}
}