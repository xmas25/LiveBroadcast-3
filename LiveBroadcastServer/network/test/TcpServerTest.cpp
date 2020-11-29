//
// Created by rjd67 on 2020/11/25.
//

#include "network/TcpServer.h"
#include "network/EventLoop.h"
#include "utils/Logger.h"

#ifdef _WIN32
NetworkInitializer init;
#endif
void OnNewMessage(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	LOG_INFO("OnNewMessage: %s", connection_ptr->GetConnectionName().c_str());
	LOG_INFO("message: \r\n%s", buffer->ReadAllAsString().c_str());
}
void OnNewConnection(const TcpConnectionPtr& connection_ptr)
{
	LOG_INFO("OnNewConnection: %s", connection_ptr->GetConnectionName().c_str());
}

int main()
{
	EventLoop loop;
	InetAddress address(4000);
	TcpServer server(&loop, "TcpServerTest", address);

	server.SetConnectionCallback(OnNewConnection);
	server.SetNewMessageCallback(OnNewMessage);

	server.Start();
	loop.Loop();
}