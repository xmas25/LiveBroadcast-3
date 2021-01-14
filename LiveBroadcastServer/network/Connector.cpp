//
// Created by rjd67 on 2021/1/14.
//

#include <memory>

#include "network/Connector.h"
#include "network/SocketOps.h"
#include "network/Channel.h"
#include "utils/Logger.h"

Connector::Connector(EventLoop* loop, const InetAddress& server_addr):
	loop_(loop),
	server_addr_(server_addr),
	status_(DISCONNECTED),
	connect_(false)
{

}

Connector::~Connector()
{
	Disconnect();
}

void Connector::Connect()
{
	connect_ = true;

	SOCKET sockfd = socketops::CreateDefaultSocket(server_addr_.GetFamily());

	int ret = socketops::Connect(sockfd, *server_addr_.GetSockAddr());
	int save_errno = (ret == 0 ? 0 : errno);

	switch (save_errno)
	{
		case 0:
		case EINTR:
		case EISCONN:
			Connecting(sockfd);
			break;
		case EAGAIN:
		case ECONNREFUSED:
			Retry(sockfd);
			break;
		case EBADF:
		case EALREADY:
			socketops::Close(sockfd);
			LOG_ERROR("connect error %d", save_errno);
			break;
		default:
			socketops::Close(sockfd);
			LOG_ERROR("connect unknown error %d", save_errno);
	}
}

void Connector::Disconnect()
{
	connect_ = false;
	if (status_ == CONNECTING)
	{
		status_ = DISCONNECTED;
		RemoveAndResetChannel();
	}
}

void Connector::SetConnectCallback(const Connector::ConnectCallback& callback)
{
	connection_callback_ = callback;
}

void Connector::ReConnect()
{
	status_ = DISCONNECTED;
	connect_ = true;
	Connect();
}

void Connector::Retry(SOCKET sockfd)
{
	socketops::Close(sockfd);
	status_ = DISCONNECTED;
	if (connect_)
	{
		LOG_INFO("Connector retry connect to %s", server_addr_.ToIpPort().c_str());
	}
	else
	{

	}
}

void Connector::Connecting(SOCKET sockfd)
{
	status_ = CONNECTING;

	/**
	 * 通过设置可写回调来验证连接是否真正建立
	 */
	channel_ptr_ = std::make_unique<Channel>(loop_, sockfd);
	channel_ptr_->SetWritableCallback([this](){HandleWrite();});
	channel_ptr_->EnableWritable();
}


SOCKET Connector::RemoveAndResetChannel()
{
	channel_ptr_->DisableAll();
	SOCKET sockfd = channel_ptr_->GetSockFd();
	channel_ptr_.reset();

	return sockfd;
}

void Connector::HandleWrite()
{
	if (status_ == CONNECTING)
	{
		/** 成功执行可写回调 连接建立 清除回调事件*/
		SOCKET sockfd = RemoveAndResetChannel();
		status_ = CONNECTED;

		/** 当连接到本机ip的时候. 由于connect会为连接选取一个本地端口 然后发起连接
		 *  如果选取的本地临时端口与连接的目标端口一致 就会出现自连接
		 * */
		if (connection_callback_)
		{
			connection_callback_(sockfd);
		}
	}

}
