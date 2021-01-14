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
	channel_.SetReadableCallback([this] { OnReadable(); });
	channel_.SetWritableCallback([this] { OnWritable(); });
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

	if (connection_callback_)
	{
		connection_callback_(shared_from_this());
	}
}

void TcpConnection::SetNewMessageCallback(const NewMessageCallback& callback)
{
	newmessage_callback_ = callback;
}

void TcpConnection::SetConnectionCallback(const ConnectionCallback& callback)
{
	connection_callback_ = callback;
}

void TcpConnection::SetWriteCompleteCallback(const WriteCompleteCallback& callback)
{
	write_complete_callback_ = callback;
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
		OnClose();
	}
	else
	{
		LOG_INFO("TcpConnection::OnReadable, error: %s", GetLastErrorAsString().c_str());
	}
}

void TcpConnection::OnWritable()
{
	if (channel_.IsWriting())
	{
		ssize_t send_size = sockfd_.Send(send_buffer_.ReadBegin(), send_buffer_.ReadableLength());

		if (send_size > 0)
		{
			send_buffer_.AddReadIndex(send_size);

			if (send_buffer_.ReadableLength() == 0)
			{
				channel_.DisableWritable();
				if (write_complete_callback_)
				{
					write_complete_callback_(shared_from_this());
				}
				/**
				 * 写结束 如果当前连接正在关闭 则可以关闭写连接
				 */
				if (connection_status_ == DISCONNECTING)
				{
					Shutdown();
				}
			}
		}
		else
		{
			LOG_ERROR("TcpConnection::OnWritable send error when writing");
		}
	}
	else
	{
		LOG_WARN("TcpConnection::OnWritable not writing");
	}
}

void TcpConnection::SetConnectionCloseCallback(const ConnectionCallback& callback)
{
	connection_close_callback_ = callback;
}

void TcpConnection::OnClose()
{
	connection_status_ = DISCONNECTED;
	channel_.DisableAll();

	if (connection_callback_)
	{
		connection_callback_(shared_from_this());
	}
	if (connection_close_callback_)
	{
		connection_close_callback_(shared_from_this());
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

void TcpConnection::Send(const char* data, size_t length)
{
	if (connection_status_ == DISCONNECTED || connection_status_ == DISCONNECTING)
	{
		LOG_WARN("disconnected, give up send");
		return;
	}

	bool send_fatal_error = false;
	ssize_t send_size = 0;
	size_t remain = length;
	/**
	 * 当前不存在未写完的数据则可以直接发送 否则Append到发送缓冲区
	 */
	if (!channel_.IsWriting() && send_buffer_.ReadableLength() == 0)
	{
		send_size = sockfd_.Send(data, length);
		if (send_size > 0)
		{
			remain -= send_size;
			if (remain == 0)
			{
				if (write_complete_callback_)
				{
					write_complete_callback_(shared_from_this());
				}
			}
		}
		else
		{
			/**
			 * EWOULDBLOCK 数据未发送出去 但是并不是致命错误
			 * 可以将数据拷贝到发送缓冲区 等待发送
			 */
			send_size = 0;
			if (errno != EWOULDBLOCK)
			{
				/**
				 * 两个非常常见的致命错误
				 * EPIPE 向关闭的连接写入数据
				 * ECONNRESET 对端关闭连接
				 */
				if (errno == EPIPE || errno == ECONNRESET)
				{
					send_fatal_error = true;
					Shutdown();
				}
			}
		}
	}

	assert(remain <= length);
	/**
	 * 未发生致命错误且数据未发送完毕
	 */
	if (!send_fatal_error && remain > 0)
	{
		send_buffer_.AppendData(&data[send_size], remain);
		if (!channel_.IsWriting())
		{
			channel_.EnableWritable();
		}
	}
}

void TcpConnection::Send(const uint8_t* data, size_t length)
{
	return Send(reinterpret_cast<const char*>(data), length);
}

void TcpConnection::Send(const Buffer* buffer)
{
	return Send(buffer->ReadBegin(), buffer->ReadableLength());
}

void TcpConnection::Send(const std::string& data)
{
	return Send(data.c_str(), data.length());
}

void TcpConnection::Shutdown()
{
	if (connection_status_ == CONNECTED)
	{
		/**
		 * 标记连接为正在关闭 但存在未写完的数据 等写完后再关闭写连接
		 */
		connection_status_ = DISCONNECTING;
		if (!channel_.IsWriting())
		{
			sockfd_.ShutdownWrite();
		}
	}
}

bool TcpConnection::HasRemainData() const
{
	return send_buffer_.ReadableLength() != 0;
}

void TcpConnection::ConnectDestroyed()
{
	if (connection_status_ == CONNECTED)
	{
		connection_status_ = DISCONNECTED;
		channel_.DisableAll();
		if (connection_callback_)
		{
			connection_callback_(shared_from_this());
		}
	}

}
