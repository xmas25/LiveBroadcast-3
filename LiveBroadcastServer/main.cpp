// LiveBroadcastServer.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <unistd.h>
#include <ctime>
#include <signal.h>

#include "network/TcpServer.h"
#include "network/EventLoop.h"
#include "utils/codec/RtmpManager.h"
#include "network/protocol/RtmpServerConnection.h"
#include "utils/Logger.h"
#include "utils/Format.h"


#ifdef _WIN32
NetworkInitializer init;
#endif

std::string ROOT = R"(/root/server/)";
std::string FILE_PREFIX = ".flv";

std::map<std::string, RtmpServerConnection*> rtmp_connection_map;
std::map<std::string, size_t> send_sub_map;

void OnShakeHandSuccess(const RtmpServerConnection* server_connection)
{

}

void OnConnection(const TcpConnectionPtr& connection_ptr)
{
	if (connection_ptr->Connected())
	{
		RtmpServerConnection* server_connection = new RtmpServerConnection(connection_ptr);
		server_connection->SetShakeHandSuccessCallback(OnShakeHandSuccess);

		/**
		 * 连接建立后 设置握手回调函数
		 */
		connection_ptr->SetNewMessageCallback(
				[server_connection](auto && PH1, auto && PH2, auto && PH3)
				{
					server_connection->OnConnectionShakeHand(PH1, PH2, PH3);
				});

		LOG_INFO("connection: %s start shake hand", connection_ptr->GetConnectionName().c_str());
	}
}

void OnNewClientMessage(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	std::string connection_data = buffer->ReadAllAsString();
	LOG_INFO("connection: %s, send\n%s", connection_ptr->GetConnectionName().c_str(),
			connection_data.c_str());

	std::string url = Format::GetUrl(connection_data);

	RtmpServerConnection* server_connection = rtmp_connection_map[url];
	if (server_connection)
	{
		server_connection->AddClientConnection(
				std::make_shared<RtmpClientConnection>(connection_ptr));
	}
}

int main()
{
	signal(SIGPIPE, SIG_IGN);
	EventLoop loop;
	InetAddress main_server_address(4000, true);
	InetAddress client_server_address(4100, true);
	TcpServer main_server(&loop, "main_server", main_server_address);
	TcpServer client_server(&loop, "client_server", client_server_address);

	main_server.SetConnectionCallback(OnConnection);
	client_server.SetNewMessageCallback(OnNewClientMessage);
	main_server.Start();
	client_server.Start();

	loop.Loop();
}
