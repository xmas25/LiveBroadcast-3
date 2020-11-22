//
// Created by rjd67 on 2020/11/22.
//
#ifdef _WIN32
#include <cassert>
#include "network/multiplexing/Select.h"
#include "network/Channel.h"

Select::Select():
	readset_(),
	writeset_(),
	readset_select_(),
	writeset_select_(),
	channel_map_()
{


}

Select::~Select()
{

}

bool Select::LoopOnce(int timeout, ChannelVector* active_channels)
{
	timeval time{0, timeout / 1000};


	readset_select_ = readset_;
	writeset_select_ = writeset_;

	int ret = select(FD_SETSIZE + 1, &readset_select_, &writeset_select_, nullptr, &time);
	if (ret == SOCKET_ERROR)
	{
		return false;
	}

	FillActiveSocketVector(ret, active_channels);
	return true;
}

void Select::UpdateChannel(Channel* channel)
{
	switch (channel->GetChannelStatus())
	{
		case Channel::CHANNEL_STATUS_NEW :
			FdSet(channel);
			channel->SetChannelStatus(Channel::CHANNEL_STATUS_ADDED);
			break;
		case Channel::CHANNEL_STATUS_ADDED:
			FdMod(channel);
			break;
		case Channel::CHANNEL_STATUS_DEL:
			FdClear(channel);
			channel->SetChannelStatus(Channel::CHANNEL_STATUS_NEW);
			break;
		default:
			break;
	}
}

void Select::FdSet(Channel* channel)
{
	uint32_t event = channel->GetEpollEvent();
	SOCKET fd = channel->GetSockFd();

	channel_map_[fd] = channel;

	if (event & XEPOLLIN)
	{
		FD_SET(fd, &readset_);
	}
	if (event & XEPOLLOUT)
	{
		FD_SET(fd, &writeset_);
	}
}

void Select::FdClear(Channel* channel)
{
	SOCKET fd = channel->GetSockFd();

	channel_map_.erase(fd);

	if (FdIsSet(fd, &readset_))
	{
		FD_CLR(fd, &readset_);
	}
	if (FdIsSet(fd, &writeset_))
	{
		FD_CLR(fd, &writeset_);
	}
}

void Select::FdMod(Channel* channel)
{
	uint32_t event = channel->GetEpollEvent();
	SOCKET fd = channel->GetSockFd();

	if (!(event & XEPOLLIN))
	{
		FD_CLR(fd, &readset_);
	}
	if (!(event & XEPOLLOUT))
	{
		FD_CLR(fd, &writeset_);
	}
}

bool Select::FdIsSet(SOCKET sockfd, fd_set* set)
{
	return FD_ISSET(sockfd, set);
}

void Select::FillActiveSocketVector(int num, ChannelVector* active_channels)
{
	uint32_t event;
	int num_temp = 0;
	for (auto&[sockfd, channel]  : channel_map_)
	{
		event = 0;
		if (FdIsSet(sockfd, &readset_select_))
		{
			event |= XEPOLLIN;
			num_temp++;
		}
		if (FdIsSet(sockfd, &writeset_select_))
		{
			event |= XEPOLLOUT;
			num_temp++;
		}
		channel->SetEvent(event);
		active_channels->push_back(channel);
	}

	assert(num == num_temp);
}

#endif