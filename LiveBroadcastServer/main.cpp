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
#include "utils/Format.h"

#ifdef _WIN32
NetworkInitializer init;
#endif

std::string ROOT = R"(C:\Users\rjd67\Desktop\Server\)";
std::string FILE_PREFIX = ".flv";

std::map<std::string, RtmpConnection*> rtmp_connection_map;

RtmpConnection* rtmp_connection = nullptr;

void OnConnection(const TcpConnectionPtr& connection_ptr)
{
	if (connection_ptr->Connected())
	{
		rtmp_connection = new RtmpConnection(connection_ptr);
		rtmp_connection_map[connection_ptr->GetConnectionName()] = rtmp_connection;

		/**
		 * 连接建立后 设置握手回调函数
		 */
		connection_ptr->SetNewMessageCallback(
				[](auto && PH1, auto && PH2, auto && PH3)
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

std::string response_header = "HTTP/1.1 200 OK\r\n"
							  "Server: ⁄(⁄ ⁄•⁄ω⁄•⁄ ⁄)⁄\r\n"
							  "Content-Type: video/x-flv\r\n"
							  "Transfer-Encoding: chunked\r\n"
							  "Connection: keep-alive\r\n"
							  "\r\n";

void OnConnectionClientServer(const TcpConnectionPtr& connection_ptr)
{
	if (connection_ptr->Connected())
	{
		ssize_t send_result = connection_ptr->Send(response_header.c_str(), response_header.length());

		if (rtmp_connection)
		{
			const Buffer* buffer = rtmp_connection->GetHeaderDataBuffer();
			std::string length_rn = Format::ToHexStringWithRN(buffer->ReadableLength());
			connection_ptr->Send(length_rn);
			connection_ptr->Send(buffer);
			connection_ptr->Send("\r\n");

			std::vector<FlvTag*> tags(*rtmp_connection->GetFlvTagVector());
			for (const auto& tag : tags)
			{
				size_t tag_size = FlvTag::FLV_TAG_HEADER_LENGTH + tag->GetBody()->ReadableLength();
				length_rn = Format::ToHexStringWithRN(tag_size);
				connection_ptr->Send(length_rn);

				connection_ptr->Send(tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
				send_result = connection_ptr->Send(tag->GetBody());
				if (send_result <= 0)
				{
					LOG_INFO("OnConnectionClientServer, error: %s",
							GetLastErrorAsString().c_str());
					break;
				}
				connection_ptr->Send("\r\n");
			}
		}
		connection_ptr->Send("0\r\n\r\n");
		connection_ptr->CloseConnection();
	}
}

int main()
{
	EventLoop loop;
	InetAddress main_server_address(4000, true);
	InetAddress client_server_address(4100, true);
	TcpServer main_server(&loop, "main_server", main_server_address);
	TcpServer client_server(&loop, "client_server", client_server_address);

	main_server.SetConnectionCallback(OnConnection);
	client_server.SetConnectionCallback(OnConnectionClientServer);
	main_server.Start();
	client_server.Start();

	loop.Loop();
}
