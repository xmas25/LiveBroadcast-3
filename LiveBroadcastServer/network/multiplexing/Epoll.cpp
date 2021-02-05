#ifndef _WIN32
#include <cerrno>
#include <algorithm>

#include "Epoll.h"
#include "network/Channel.h"
#include "utils/Logger.h"

Epoll::Epoll() :
	epfd_(::epoll_create(5)),
	event_vector_(50)
{

}

bool Epoll::LoopOnce(int timeout, ChannelVector* active_channels)
{
	int ret = ::epoll_wait(epfd_, event_vector_.data(), event_vector_.size(), timeout);

	if (ret == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			return true;
		}
		else if (errno == EINTR)
		{
			return true;
		}
		LOG_ERROR("error signal: %d", errno);

		return false;
	}
	else if (ret == 0)
	{
		return true;
	}
	else
	{
		FillActiveSocketVector(ret, active_channels);
		return true;
	}
}


void Epoll::UpdateChannel(Channel* channel)
{
	switch (channel->GetChannelStatus())
	{
	case Channel::CHANNEL_STATUS_NEW :
		UpdateChannelInternal(EPOLL_CTL_ADD, channel);
		break;
	case Channel::CHANNEL_STATUS_ADDED:
		UpdateChannelInternal(EPOLL_CTL_MOD, channel);
		break;
	case Channel::CHANNEL_STATUS_DEL:
		RemoveChannel(channel);
		break;
	default:
		break;
	}
}


void Epoll::FillActiveSocketVector(int nums, ChannelVector* active_channels)
{
	for (int i = 0; i < nums; ++i)
	{
		epoll_event* ev = &event_vector_[i];
		Channel* channel = reinterpret_cast<Channel*>(ev->data.ptr);

		if (channel)
		{
			channel->SetEvent(ev->events);
			active_channels->push_back(channel);
		}
	}
}

void Epoll::EpollControl(int ctl, SOCKET fd, void* ptr, uint32_t events)
{
	if (ctl == EPOLL_CTL_DEL)
	{
		epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
	}
	else
	{
		epoll_event ev;
		ev.data.ptr = ptr;
		ev.events = events;
		epoll_ctl(epfd_, ctl, fd, &ev);
	}
}

void Epoll::RemoveChannel(Channel* channel)
{
	SOCKET fd = channel->GetSockFd();

	epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);

	channel->SetChannelStatus(Channel::CHANNEL_STATUS_NEW);
}

void Epoll::UpdateChannelInternal(int ctl, Channel* channel)
{
	epoll_event ev;
	ev.data.ptr = channel;
	ev.events = channel->GetEpollEvent();
	SOCKET fd = channel->GetSockFd();

	if (ctl == EPOLL_CTL_ADD)
	{
		channel->SetChannelStatus(Channel::CHANNEL_STATUS_ADDED);
	}
	else if (ctl == EPOLL_CTL_MOD)
	{

	}
	epoll_ctl(epfd_, ctl, fd, &ev);
}

#endif