#include "network/TcpConnection.h"
#include "utils/Logger.h"

TcpConnection::TcpConnection(EventLoop* loop, const std::string& connection_name, SOCKET sockfd,
		const InetAddress& address) :
		connection_status_(CONNECTING),
		loop_(loop),
		connection_name_(connection_name),
		sockfd_(sockfd),
		channel_(loop, connection_name, sockfd),
		address_(address)
{
	channel_.SetReadableCallback(std::bind(&TcpConnection::OnReadable, this));
}

TcpConnection::~TcpConnection()
{
	LOG_INFO("Connection: %s deconstruct", connection_name_.c_str());
}

void TcpConnection::Established()
{
	connection_status_ = CONNECTED;

	channel_.TieConnection(shared_from_this());
	channel_.EnableReadable();
	connection_callback_(shared_from_this());
}

void TcpConnection::SetNewMessageCallback(const NewMessageCallback& callback)
{
	newmessage_callback_ = callback;
}

void TcpConnection::SetConnectionCallback(const ConnectionCallback& callback)
{
	connection_callback_ = callback;
}

void TcpConnection::OnReadable()
{
	ssize_t result = recv_buffer_.ReadFromSockfd(sockfd_.GetSockFd());
	if (result > 0)
	{
		if (newmessage_callback_)
		{
			Timestamp timestamp;
			newmessage_callback_(shared_from_this(), &recv_buffer_, timestamp);
		}
	}
	else if (result == 0)
	{
		/**
		 * 首先断开channel和多路复用的连接
		 * 然后断开和TcpServer的连接
		 */
		CloseConnection();
	}
	else
	{
		abort();

	}
}

void TcpConnection::SetConnectionCloseCallback(const ConnectionCallback& callback)
{
	connection_close_callback_ = callback;
}

void TcpConnection::CloseConnection()
{
	connection_status_ = DISCONNECTED;
	channel_.DisableAll();
	if (connection_close_callback_)
	{
		connection_close_callback_(shared_from_this());
	}
	if (connection_callback_)
	{
		connection_callback_(shared_from_this());
	}
}

const std::string& TcpConnection::GetConnectionName() const
{
	return connection_name_;
}

SOCKET TcpConnection::GetSockfd() const
{
	return sockfd_.GetSockFd();
}

bool TcpConnection::Connected() const
{
	return connection_status_ == CONNECTED;
}
