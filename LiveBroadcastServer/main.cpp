// LiveBroadcastServer.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <unistd.h>
#include <ctime>

#include "network/TcpServer.h"
#include "network/EventLoop.h"
#include "utils/codec/RtmpManager.h"
#include "network/protocol/RtmpConnection.h"
#include "utils/Logger.h"

#ifdef _WIN32
NetworkInitializer init;
#endif

std::string ROOT = R"(C:\Users\rjd67\Desktop\Server\)";
std::string FILE_PREFIX = ".flv";

std::map<std::string, RtmpConnection*> rtmp_connection_map;

void OnConnection(const TcpConnectionPtr& connection_ptr)
{
	if (connection_ptr->Connected())
	{
		rtmp_connection_map[connection_ptr->GetConnectionName()] = new RtmpConnection(connection_ptr);
		
		LOG_INFO("connection: %s start shake hand", connection_ptr->GetConnectionName().c_str());
	}
	else
	{
		RtmpConnection* rtmp_manager = rtmp_connection_map[connection_ptr->GetConnectionName()];

		time_t t = time(nullptr);
		File file_write(ROOT + std::to_string(t) + FILE_PREFIX, File::O_WRONLY);
		rtmp_manager->WriteToFile(&file_write);
	}
}

void OnNewMessage(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	RtmpConnection* rtmp_manager = rtmp_connection_map[connection_ptr->GetConnectionName()];

	rtmp_manager->ParseData(buffer);
}

void OnConnectionShakeHand(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	RtmpConnection* rtmp_manager = rtmp_connection_map[connection_ptr->GetConnectionName()];

	RtmpConnection::ShakeHandResult result = rtmp_manager->ShakeHand(buffer);
	switch (result)
	{
		case RtmpConnection::SHAKE_SUCCESS:
		{
			connection_ptr->SetNewMessageCallback(OnNewMessage);
			LOG_INFO("connection: %s shake hand success",
					connection_ptr->GetConnectionName().c_str());
			/**
			 * 握手成功时返回
			 */
			return;
		}
		case RtmpConnection::SHAKE_FAILED:
		{
			LOG_WARN("connection: %s shake hand failed",
					connection_ptr->GetConnectionName().c_str());
			connection_ptr->CloseConnection();
			/**
			 * 出错时返回
			 */
			return;
		}
		case RtmpConnection::SHAKE_DATA_NOT_ENOUGH:
			/**
			 * 数据不足时返回
			 */
			return;
	}
}

int main()
{
	EventLoop loop;
	InetAddress address(4000, true);
	TcpServer server(&loop, "main_server", address);
	server.SetConnectionCallback(OnConnection);
	server.SetNewMessageCallback(OnConnectionShakeHand);
	server.Start();

	loop.Loop();
}
