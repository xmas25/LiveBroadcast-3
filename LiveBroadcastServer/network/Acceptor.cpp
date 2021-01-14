#include "network/Acceptor.h"
#include "network/SocketOps.h"

void DefaultNewConnection(SOCKET fd, const InetAddress& address)
{
	
}

Acceptor::Acceptor(EventLoop* loop, const std::string& server_name, const InetAddress& address) :
	loop_(loop),
	name_(server_name + '-' + "acceptor"),
	listenfd_(socketops::CreateDefaultSocket(address.GetFamily())),
	channel_(loop, name_, listenfd_.GetSockFd()),
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

SOCKET Acceptor::Accept(InetAddress* address)
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
	SOCKET fd = Accept(&address);

	if (newconnection_callback_)
	{
		newconnection_callback_(fd, address);
	}
}