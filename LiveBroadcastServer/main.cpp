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
		RtmpConnection* rtmp_connection = new RtmpConnection(connection_ptr);
		rtmp_connection_map[connection_ptr->GetConnectionName()] = rtmp_connection;

		connection_ptr->SetNewMessageCallback(
				[rtmp_connection](auto && PH1, auto && PH2, auto && PH3)
				{
					rtmp_connection->OnConnectionShakeHand(PH1, PH2, PH3);
				});

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

int main()
{
	EventLoop loop;
	InetAddress main_server_address(4000, true);
	InetAddress client_server_address(4100, true);
	TcpServer main_server(&loop, "main_server", main_server_address);
	// TcpServer client_server(&loop, "client_server", client_server_address);

	main_server.SetConnectionCallback(OnConnection);
	main_server.Start();

	loop.Loop();
}
