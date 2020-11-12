#include <unistd.h>

#include "network/SocketOps.h"
#include "network/Socket.h"

Socket::Socket(int sockfd) :
	sockfd_(sockfd)
{

}

Socket::~Socket()
{
	if (sockfd_ > 0)
	{
		close(sockfd_);
		sockfd_ = -1;
	}
}

int Socket::GetSockFd() const
{
	return sockfd_;
}

void Socket::SetSockfd(int sockfd)
{
	sockfd_ = sockfd;
}

void Socket::SetReusePort()
{
	socketops::SetReusePort(sockfd_);
}

void Socket::SetReuseAddr()
{
	socketops::SetReuseAddr(sockfd_);
}

void Socket::Bind(const InetAddress& address)
{
	socketops::Bind(sockfd_, *address.GetSockAddr());
}

void Socket::Listen()
{
	socketops::Listen(sockfd_);
}

int Socket::Accept(InetAddress* address)
{

	return socketops::Accept(sockfd_, address->GetSockAddr());
}

