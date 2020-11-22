// LiveBroadcastServer.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <unistd.h>
#include <ctime>

#include "network/TcpServer.h"
#include "utils/codec/RtmpManager.h"

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

void WriteToFile(FlvManager* flv_manager, File* file_write)
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
	printf("sum write: %zu\n", sum_write_bytes);
	printf("write file: %s success\n", file_write->GetPath().c_str());
}

void NewConnection(SOCKET fd, const InetAddress& address)
{
	ShakeHands(fd);

	RtmpManager rtmp_manager;
	Buffer buffer(8192);
	while (true)
	{
		ssize_t read_bytes = buffer.ReadFromSockfd(fd);

		if (read_bytes == 0)
		{
			printf("connection close\n");
			close(fd);
			break;
		}
		rtmp_manager.ParseData(&buffer);
	}

	time_t t = time(nullptr);
	File file(ROOT + std::to_string(t) + FILE_PREFIX, File::O_WRONLY);
	WriteToFile(rtmp_manager.GetFlvManager(), &file);
}


int main()
{
	EventLoop loop;
	InetAddress address(4000, true);
	TcpServer server(&loop, address);
	server.SetNewConnectionCallback(NewConnection);
	server.Start();

	loop.Loop();
}
