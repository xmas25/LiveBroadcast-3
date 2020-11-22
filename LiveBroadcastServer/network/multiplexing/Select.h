//
// Created by rjd67 on 2020/11/22.
//
#ifdef _WIN32
#ifndef LIVEBROADCASTSERVER_SELECT_H
#define LIVEBROADCASTSERVER_SELECT_H

#include <winsock2.h>
#include <map>
#include "network/multiplexing/MultiplexingBase.h"
#include "network/Callback.h"

class Channel;
class Select : public MultiplexingBase
{
public:

	Select();

	~Select() override;

	/**
	 * 循环一次
	 * @param timeout 超时时间 单位毫秒
	 * @param active_channels 用于返回产生事件的channel
	 * @return
	 */
	bool LoopOnce(int timeout, ChannelVector* active_channels) override;

	void UpdateChannel(Channel* channel) override;

private:

	fd_set readset_;
	fd_set writeset_;

	fd_set readset_select_;
	fd_set writeset_select_;

	std::map<SOCKET, Channel*> channel_map_;

	void FdSet(Channel* channel);
	void FdClear(Channel* channel);
	void FdMod(Channel* channel);
	bool FdIsSet(SOCKET sockfd, fd_set* set);

	void FillActiveSocketVector(int num, ChannelVector* active_channels);
};

#endif //LIVEBROADCASTSERVER_SELECT_H
#endif