// LiveBroadcastServer.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <unistd.h>
#include <ctime>

#include "network/TcpServer.h"
#include "network/EventLoop.h"
#include "utils/codec/RtmpManager.h"
#include "utils/Logger.h"

#ifdef _WIN32
NetworkInitializer init;
#endif

std::string ROOT = R"(C:\Users\rjd67\Desktop\Server\)";
std::string FILE_PREFIX = ".flv";

void ShakeHands(SOCKET fd)
{
	Buffer buffer(8192);

	ssize_t read_bytes = buffer.ReadFromSockfdAndDrop(fd);
	std::cout << GetLastErrorAsString() << std::endl;
	assert(read_bytes == 1537);

	ssize_t send_bytes = write(fd, RTMP_S01, sizeof RTMP_S01);
	assert(send_bytes == 1537);

	read_bytes = buffer.ReadFromSockfdAndDrop(fd);
	assert(read_bytes == 1536);

	send_bytes = write(fd, RTMP_S2, sizeof RTMP_S2);
	assert(send_bytes == 1536);

	read_bytes = buffer.ReadFromSockfdAndDrop(fd);
	write(fd, RTMP_1, sizeof RTMP_1);

	read_bytes = buffer.ReadFromSockfdAndDrop(fd);
	write(fd, RTMP_2, sizeof RTMP_2);
	write(fd, RTMP_3, sizeof RTMP_3);
	write(fd, RTMP_4, sizeof RTMP_4);

	read_bytes = buffer.ReadFromSockfdAndDrop(fd);
	write(fd, RTMP_5, sizeof RTMP_5);

	read_bytes = buffer.ReadFromSockfdAndDrop(fd);
	write(fd, RTMP_6, sizeof RTMP_6);
}

ssize_t WriteToFile(FlvManager* flv_manager, File* file_write)
{
	/**
	 * 写文件头 包含header, 脚本Tag, 第一个音频和第一个视频Tag
	 */
	Buffer buffer;
	flv_manager->EncodeHeadersToBuffer(&buffer);
	file_write->Write(&buffer);


	/**
	 * 写文件体
	 */
	std::vector<FlvTag*>* flv_tags = flv_manager->GetFlvTags();

	ssize_t write_bytes;
	ssize_t sum_write_bytes = buffer.GetSumWrite();
	for (FlvTag* tag : *flv_tags)
	{
		write_bytes = file_write->Write(tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
		assert(write_bytes == FlvTag::FLV_TAG_HEADER_LENGTH);
		sum_write_bytes += write_bytes;

		const std::string* body = tag->GetBody()->GetBody();
		write_bytes = file_write->Write(body->data(), body->length());
		assert(write_bytes == static_cast<ssize_t>(body->length()));
		sum_write_bytes += write_bytes;
	}
	
	return sum_write_bytes;
}

std::map<std::string, RtmpManager*> rtmp_manager_map;
std::map<std::string, size_t> last_write_m;

void OnConnection(const TcpConnectionPtr& connection_ptr)
{
	if (connection_ptr->Connected())
	{
		int fd = connection_ptr->GetSockfd();
		ShakeHands(fd);

		rtmp_manager_map[connection_ptr->GetConnectionName()] = new RtmpManager;
		last_write_m[connection_ptr->GetConnectionName()] = 0;
		
		LOG_INFO("connection: %s start parse", connection_ptr->GetConnectionName().c_str());
	}
	else
	{
		LOG_INFO("connection: %s start write data to file", connection_ptr->GetConnectionName().c_str());
		
		RtmpManager* rtmp_manager = rtmp_manager_map[connection_ptr->GetConnectionName()];

		time_t t = time(nullptr);
		File file_write(ROOT + std::to_string(t) + FILE_PREFIX, File::O_WRONLY);
		ssize_t sum_write_bytes = WriteToFile(rtmp_manager->GetFlvManager(), &file_write);
		
		LOG_INFO("connection: %s write %zob bytes to %s success",
				connection_ptr->GetConnectionName().c_str(),
				sum_write_bytes,
				file_write.GetPath().c_str());
	}
}

void OnNewMessage(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	RtmpManager* rtmp_manager = rtmp_manager_map[connection_ptr->GetConnectionName()];

	rtmp_manager->ParseData(buffer);

	size_t read_m = buffer->GetSumRead() / 1000000;
	if (read_m != last_write_m[connection_ptr->GetConnectionName()])
	{
		last_write_m[connection_ptr->GetConnectionName()] = read_m;
		LOG_INFO("connection: %s, sum write %zuMb", connection_ptr->GetConnectionName().c_str(),
				read_m);
	}
}

int main()
{
	EventLoop loop;
	InetAddress address(4000, true);
	TcpServer server(&loop, "main_server", address);
	server.SetConnectionCallback(OnConnection);
	server.SetNewMessageCallback(OnNewMessage);
	server.Start();

	loop.Loop();
}
