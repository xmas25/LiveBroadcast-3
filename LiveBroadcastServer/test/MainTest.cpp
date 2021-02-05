//
// Created by rjd67 on 2021/2/5.
//
#include "network/TcpClient.h"
#include "network/TcpConnection.h"
#include "utils/File.h"

/**
 * 2.data.back
 *
 * 包含Obs从点击开始推流到正常音视频数据的二进制数据
 * */

File file("/root/2.data.back", File::O_RDONLY);
Buffer buffer;

void OnWriteOver(const TcpConnectionPtr& connection)
{
	file.Read(&buffer);

	connection->Send(&buffer);
}

void OnConnect(const TcpConnectionPtr& connection)
{
	file.Read(&buffer);

	connection->Send(&buffer);
}

int main()
{
	EventLoop loop;
	InetAddress address(4000);
	TcpClient client(&loop, address, "MainTest");

	client.SetConnectCallback(OnConnect);
	client.SetWriteCompleteCallback(OnWriteOver);

	client.Connect();
	loop.Loop();
}