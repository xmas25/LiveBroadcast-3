#include "network/TcpConnection.h"

TcpConnection::TcpConnection(EventLoop* loop, const std::string& connection_name, SOCKET sockfd,
		const InetAddress& address) :
		loop_(loop),
		connection_name_(connection_name),
		socket_(sockfd),
		channel_(loop, sockfd),
		address_(address)
{
	channel_.SetReadableCallback(std::bind(&TcpConnection::OnReadable, this));
}

void TcpConnection::Established()
{
	channel_.TieConnection(shared_from_this());
	channel_.EnableReadable();
	newconnection_callback_(shared_from_this());
}

void TcpConnection::SetNewMessageCallback(const NewMessageCallback& callback)
{
	newmessage_callback_ = callback;
}

void TcpConnection::SetNewConnectionCallback(const NewConnectionCallback& callback)
{
	newconnection_callback_ = callback;
}

void TcpConnection::OnReadable()
{
	ssize_t result = recv_buffer_.ReadFromSockfd(socket_.GetSockFd());
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
		if (connection_close_callback_)
		{
			connection_close_callback_(shared_from_this());
		}
	}
	else
	{
		abort();
	}
}

void TcpConnection::SetConnectionCloseCallback(const ConnectionCloseCallback& callback)
{
	connection_close_callback_ = callback;
}

void TcpConnection::CloseConnection()
{
	channel_.DisableAll();
}

const std::string& TcpConnection::GetConnectionName() const
{
	return connection_name_;
}
