#include "network/TcpServer.h"
#include "network/EventLoop.h"

TcpServer::TcpServer(EventLoop* loop, const InetAddress& address) :
	loop_(loop),
	acceptor_(loop, address)
{
	
}

TcpServer::~TcpServer()
{
}

void TcpServer::Start()
{
	acceptor_.Listen();
}

void TcpServer::SetNewConnectionCallback(const NewConnectionCallback& cb)
{
	acceptor_.SetNewConnectionCallback(cb);
}
