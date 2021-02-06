// LiveBroadcastServer.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <csignal>

#include "network/TcpServer.h"
#include "network/EventLoop.h"
#include "utils/codec/RtmpManager.h"
#include "network/protocol/RtmpServerConnection.h"
#include "utils/Logger.h"
#include "utils/Format.h"
#include "mapper/UserMapper.h"

#ifdef _WIN32
NetworkInitializer init;
#endif

std::map<std::string, RtmpServerConnection*> rtmp_connection_map;
// UserMapper user_mapper_;

bool OnAuthenticate(const std::string& user, const std::string& passwd)
{
	return true;
//	if (user_mapper_.GetPasswdByUser(user) == passwd)
//	{
//		return true;
//	}
//	else
//	{
//		LOG_WARN("user: %s, use wrong passwd: %s", user.c_str(), passwd.c_str())
//		return false;
//	}

}

void OnShakeHandSuccess(RtmpServerConnection* server_connection)
{
	/**
	 * 将推流的url和server_connection关联起来 用于拉流的时候根据url获取对应的server_connection
	 *
	 * rtmp到server_connection 映射关系可以自己修改
	 */
	std::string path = server_connection->GetRtmpPath();
	rtmp_connection_map[path] = server_connection;
	LOG_INFO("server: %s bind to %s", server_connection->GetConnectionName().c_str(),
			path.c_str());
}

/** 主播建立连接后的回调函数*/
void OnConnection(const TcpConnectionPtr& connection_ptr)
{
	if (connection_ptr->Connected())
	{
		RtmpServerConnection* server_connection = new RtmpServerConnection(connection_ptr);

		// 连接建立后RtmpServerConnection内部会进行握手 然后握手成功后调用函数
		server_connection->SetShakeHandSuccessCallback(OnShakeHandSuccess);
		server_connection->SetAuthenticationCallback(OnAuthenticate);

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

void OnClientMessage(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	std::string connection_data = buffer->ReadAllAsString();

	/**
	 * 获取HTTP请求中的url 根据上面设置的映射关系 同样获取url
	 * 来获取到对应的server_connection 加入其中
	 */
	std::string url = Format::GetUrl(connection_data);
	RtmpServerConnection* server_connection = rtmp_connection_map[url];

	if (server_connection)
	{
		LOG_INFO("connection: %s, request url: %s success", connection_ptr->GetConnectionName().c_str(),
				url.c_str());
		server_connection->AddClientConnection(
				std::make_shared<RtmpClientConnection>(connection_ptr));
	}
	else
	{
		LOG_INFO("connection: %s, request url: %s failed", connection_ptr->GetConnectionName().c_str(),
				url.c_str());
		connection_ptr->Shutdown();
	}
}

int main(int argc, char* argv[])
{

#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

	if (argc != 3)
	{
		printf("wrong number of parameters\r\n");
		exit(-1);
	}

//	if (!user_mapper_.Initialize(
//			"127.0.0.1", "lsmg", "123456789", "live"))
//	{
//		exit(-1);
//	}


	short main_server_port = atoi(argv[1]);
	short client_server_port = atoi(argv[2]);

	EventLoop loop;
	InetAddress main_server_address(main_server_port, true);
	InetAddress client_server_address(client_server_port, true);
	TcpServer main_server(&loop, "main_server", main_server_address);
	TcpServer client_server(&loop, "client_server", client_server_address);

	main_server.SetConnectionCallback(OnConnection);
	client_server.SetNewMessageCallback(OnClientMessage);
	main_server.Start();
	client_server.Start();

	loop.Loop();
}
