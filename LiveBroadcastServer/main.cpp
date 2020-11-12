// LiveBroadcastServer.cpp : Defines the entry point for the application.
//
#include <iostream>
#include "network/TcpServer.h"

#include <unistd.h>

std::string hello_world = "hello world!";

void NewConnection(int fd, const InetAddress& address)
{
	write(fd, hello_world.data(), hello_world.length());
	close(fd);
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
