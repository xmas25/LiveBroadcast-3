#include "network/Acceptor.h"
#include "network/SocketOps.h"

void DefaultNewConnection(int fd, const InetAddress& address)
{
	
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& address) :
	loop_(loop),
	listenfd_(socketops::Socket(address.GetFamily())),
	channel_(loop, listenfd_.GetSockFd()),
	newconnection_callback_(DefaultNewConnection)
{
	listenfd_.SetReuseAddr();
	listenfd_.Bind(address);

	channel_.SetReadableCallback(std::bind(&Acceptor::OnReadable, this));
	channel_.EnableReadable();

}

Acceptor::~Acceptor()
{
}

void Acceptor::Listen()
{
	listenfd_.Listen();
}

void Acceptor::SetNewConnectionCallback(const NewConnectionCallback& cb)
{
	newconnection_callback_ = cb;
}

int Acceptor::Accept(InetAddress* address)
{
	if (!address)
	{
		return -1;
	}
	return listenfd_.Accept(address);
}

void Acceptor::OnReadable()
{
	InetAddress address;
	int fd = Accept(&address);

	if (newconnection_callback_)
	{
		newconnection_callback_(fd, address);
	}
}