#include "network/EventLoop.h"
#include "network/Channel.h"
#include "network/multiplexing/MultiplexingBase.h"
#include "network/multiplexing/Epoll.h"
#include "network/multiplexing/Select.h"

EventLoop::EventLoop() :
	multiplexing_base_(nullptr),
	active_channels_(),
	looping_(false)
{
#ifdef _WIN32
	multiplexing_base_ = new Select;
#else
	multiplexing_base_ = new Epoll;
#endif
}

EventLoop::~EventLoop()
{
	delete multiplexing_base_;
}

void EventLoop::Loop()
{
	looping_ = true;

	while (looping_)
	{
		active_channels_.clear();
		looping_ = multiplexing_base_->LoopOnce(20, &active_channels_);

		HandleActiveChannel();
		HandlePendingFunc();
	}
}

void EventLoop::Update(Channel* channel)
{
	multiplexing_base_->UpdateChannel(channel);
}

void EventLoop::HandleActiveChannel()
{

	for (Channel* channel : active_channels_)
	{
		channel->HandleEventWithGuard();
	}
}

void EventLoop::RunInLoop(const EventLoop::EventLoopFunction& function)
{
	pending_func_.push_back(function);
}

void EventLoop::HandlePendingFunc()
{
	for (auto& func : pending_func_)
	{
		func();
	}
	pending_func_.clear();
}
