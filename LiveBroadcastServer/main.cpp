// LiveBroadcastServer.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <unistd.h>
#include <ctime>
#include <signal.h>

#include "network/TcpServer.h"
#include "network/EventLoop.h"
#include "utils/codec/RtmpManager.h"
#include "network/protocol/RtmpConnection.h"
#include "utils/Logger.h"
#include "utils/Format.h"

#ifdef _WIN32
NetworkInitializer init;
#endif

std::string ROOT = R"(/root/server/)";
std::string FILE_PREFIX = ".flv";

std::map<std::string, RtmpConnection*> rtmp_connection_map;
std::map<std::string, size_t> send_sub_map;

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

		// delete rtmp_connection;
		// rtmp_connection = nullptr;
	}
}


void OnConnectionWritable(const TcpConnectionPtr& connection_ptr)
{
	if (rtmp_connection)
	{
		size_t send_sub = send_sub_map[connection_ptr->GetConnectionName()];
		const std::vector<FlvTag*>* tags = rtmp_connection->GetFlvTagVector();
		if (send_sub < tags->size())
		{
			Buffer send_buffer;
			const FlvTag* tag = (*tags)[send_sub];
			size_t tag_size = FlvTag::FLV_TAG_HEADER_LENGTH + tag->GetBody()->ReadableLength();
			std::string length_rn = Format::ToHexStringWithCrlf(tag_size);
			send_buffer.AppendData(length_rn);
			send_buffer.AppendData(tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
			send_buffer.AppendData(tag->GetBody());
			send_buffer.AppendData("\r\n");
			send_sub_map[connection_ptr->GetConnectionName()]++;

			connection_ptr->Send(&send_buffer);
		}
		else
		{
			send_sub_map[connection_ptr->GetConnectionName()] = 0;
			connection_ptr->SetWriteCompleteCallback(nullptr);
			connection_ptr->Send("0\r\n\r\n");
		}
	}
}

void OnConnectionClientServer(const TcpConnectionPtr& connection_ptr)
{

}

std::string response_header = "HTTP/1.1 200 OK\r\n"
							  "Server: FISH_LIVE\r\n"
							  "Date: Sun, 29 Nov 2020 15:30:42 GMT\r\n"
							  "Content-Type: video/x-flv\r\n"
							  "Transfer-Encoding: chunked\r\n"
							  "Connection: keep-alive\r\n"
		  					  "Access-Control-Allow-Origin: *\r\n\r\n";
void OnNewClientMessage(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	LOG_INFO("connection: %s, send\n%s", connection_ptr->GetConnectionName().c_str(),
			buffer->ReadAllAsString().c_str());

	if (rtmp_connection)
	{
		if (connection_ptr->Connected())
		{
			/**
			 *发送HTTP头和 FlV数据的头部
			 */

			send_sub_map[connection_ptr->GetConnectionName()] = 0;

			const Buffer* header_buffer = rtmp_connection->GetHeaderDataBuffer();
			std::string length_rn = Format::ToHexStringWithCrlf(header_buffer->ReadableLength());

			Buffer send_buffer;
			send_buffer.AppendData(response_header);
			send_buffer.AppendData(length_rn);
			send_buffer.AppendData(header_buffer);
			send_buffer.AppendData("\r\n");

			connection_ptr->Send(&send_buffer);
		}
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
	client_server.SetConnectionCallback(OnConnectionClientServer);
	client_server.SetNewMessageCallback(OnNewClientMessage);
	client_server.SetWriteCompleteCallback(OnConnectionWritable);
	main_server.Start();
	client_server.Start();

	loop.Loop();
}
