﻿// LiveBroadcastServer.cpp : Defines the entry point for the application.
//
#include <iostream>
#include "network/TcpServer.h"
#include "server/codec/RtmpCodec.h"
#include "utils/File.h"

#include <unistd.h>

std::string hello_world = "hello world!";

void NewConnection(int fd, const InetAddress& address)
{
	char buffer[2048];
	
	ssize_t read_bytes = read(fd, buffer, sizeof buffer);
	assert(read_bytes == 1537);

	ssize_t send_bytes = write(fd, RTMP_S01, sizeof RTMP_S01);
	assert(send_bytes == 1537);

	read_bytes = read(fd, buffer, sizeof buffer);
	assert(read_bytes == 1536);

	send_bytes = write(fd, RTMP_S2, sizeof RTMP_S2);
	assert(send_bytes == 1536);

	read_bytes = read(fd, buffer, sizeof buffer);
	write(fd, RTMP_1, sizeof RTMP_1);

	read_bytes = read(fd, buffer, sizeof buffer);
	write(fd, RTMP_2, sizeof RTMP_2);
	write(fd, RTMP_3, sizeof RTMP_3);
	write(fd, RTMP_4, sizeof RTMP_4);

	read_bytes = read(fd, buffer, sizeof buffer);
	write(fd, RTMP_5, sizeof RTMP_5);

	read_bytes = read(fd, buffer, sizeof buffer);
	write(fd, RTMP_6, sizeof RTMP_6);


	File file("/root/server/1.data", File::O_WRONLY);
	
	while (true)
	{
		read_bytes = read(fd, buffer, sizeof buffer);
		if (read_bytes <= 0)
		{
			close(fd);
			break;
		}

		send_bytes = file.Write(buffer, read_bytes);
		assert(send_bytes == read_bytes);
	}
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