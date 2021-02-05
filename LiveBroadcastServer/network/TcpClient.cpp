//
// Created by rjd67 on 2021/1/14.
//

#include "network/TcpClient.h"
#include "network/SocketOps.h"
#include "network/TcpConnection.h"
#include "utils/Logger.h"

void DefaultOnMessage(const TcpConnectionPtr& connection, Buffer* buffer, Timestamp timestamp)
{
	buffer->Reset();
}

TcpClient::TcpClient(EventLoop* loop, const InetAddress& server_addr, const std::string& client_name):
	loop_(loop),
	server_addr_(server_addr),
	client_name_(client_name),
	newmessage_callback_(DefaultOnMessage),
	connector_(loop, server_addr),
	connection_id_(0),
	retry_(false),
	connect_(false)
{
	connector_.SetConnectCallback([this](auto&& PH1){OnNewConnection(PH1);});
}

TcpClient::~TcpClient()
{
	DisConnect();
}

void TcpClient::SetNewMessageCallback(const NewMessageCallback& callback)
{
	newmessage_callback_ = callback;
}

void TcpClient::SetConnectCallback(const ConnectionCallback& callback)
{
	connection_callback_ = callback;
}

void TcpClient::SetWriteCompleteCallback(const ConnectionCallback& callback)
{
	write_complete_callback_ = callback;
}

void TcpClient::Connect()
{
	connect_ = true;
	loop_->RunInLoop([this](){connector_.Connect();});
}

void TcpClient::DisConnect()
{
	connect_ = false;
	RemoveConnection(connection_ptr_);
}

void TcpClient::SetReConnect(bool open)
{
	retry_ = true;
}

void TcpClient::OnNewConnection(SOCKET sockfd)
{
	InetAddress address(socketops::GetPeerAddr(sockfd));

	char buffer[64];
	snprintf(buffer, sizeof buffer, "-%s#%d", address.ToIpPort().c_str(), connection_id_++);
	std::string connection_name = client_name_ + buffer;

	TcpConnectionPtr connection_ptr = std::make_shared<TcpConnection>(loop_, connection_name,
			sockfd, address);

	connection_ptr->SetNewMessageCallback(newmessage_callback_);
	connection_ptr->SetWriteCompleteCallback(write_complete_callback_);
	connection_ptr->SetConnectionCallback(connection_callback_);
	connection_ptr->SetConnectionCloseCallback([this](auto&& PH1){RemoveConnection(PH1);});
	// connection_ptr->SetConnectionCloseCallback(std::bind(&TcpClient::RemoveConnection, this, _1));

	LOG_INFO("create a new connection: %s", connection_name.c_str());

	connection_ptr_ = connection_ptr;
	connection_ptr->Established(); // TODO Run in loop
}

void TcpClient::RemoveConnection(const TcpConnectionPtr& connection_ptr)
{
	if (connection_ptr == connection_ptr_)
	{
		connection_ptr_.reset();

		connection_ptr->ConnectDestroyed();

		if (retry_ && connect_)
		{
			LOG_INFO("TcpClient[%s] - ReConnecting to %s", client_name_.c_str(),
					server_addr_.ToIpPort().c_str());
			connector_.ReConnect();
		}
	}
	else
	{
		LOG_ERROR("Error")
	}
}
