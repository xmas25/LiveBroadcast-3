#include "network/TcpServer.h"
#include "network/EventLoop.h"
#include "utils/Logger.h"

TcpServer::TcpServer(EventLoop* loop, const std::string& server_name, const InetAddress& address) :
	loop_(loop),
	server_name_(server_name),
	server_address_(address),
	acceptor_(loop, server_name, address),
	connection_id_(0)
{
	acceptor_.SetNewConnectionCallback(std::bind(&TcpServer::OnNewConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
	for (auto [connection_name, connection_ptr] : connection_map_)
	{
		LOG_INFO("close connection: %s", connection_name.c_str());
		// connection_ptr->Destroy();
	}
}

void TcpServer::Start()
{
	LOG_INFO("server: %s listen on %s",
			server_name_.c_str(), server_address_.ToIpPort().c_str())
	acceptor_.Listen();
}

void TcpServer::SetConnectionCallback(const ConnectionCallback& cb)
{
	connection_callback_ = cb;
}

void TcpServer::SetWriteCompleteCallback(const WriteCompleteCallback& cb)
{
	write_complete_callback_ = cb;
}

void TcpServer::OnNewConnection(SOCKET sockfd, const InetAddress& address)
{
	char buffer[64];
	snprintf(buffer, sizeof buffer, "-%s#%d", address.ToIpPort().c_str(), connection_id_++);
	std::string connection_name = server_name_ + buffer;

	TcpConnectionPtr connection_ptr = std::make_shared<TcpConnection>(loop_, connection_name,
			sockfd, address);
	connection_ptr->SetNewMessageCallback(newmessage_callback_);
	connection_ptr->SetWriteCompleteCallback(write_complete_callback_);
	connection_ptr->SetConnectionCallback(connection_callback_);
	connection_ptr->SetConnectionCloseCallback(std::bind(&TcpServer::OnCloseConnection,this, _1));

	connection_map_[connection_name] = connection_ptr;

	LOG_INFO("create a new connection: %s", connection_name.c_str());

	connection_ptr->Established(); // TODO Run in loop
}

void TcpServer::SetNewMessageCallback(const NewMessageCallback& callback)
{
	newmessage_callback_ = callback;
}

void TcpServer::OnCloseConnection(const TcpConnectionPtr& connection_ptr)
{
	std::string connection_name = connection_ptr->GetConnectionName();

	LOG_INFO("erase a connection: %s", connection_name.c_str());
	connection_map_.erase(connection_name);
}
