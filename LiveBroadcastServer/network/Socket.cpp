#include <unistd.h>

#include "network/SocketOps.h"
#include "network/Socket.h"

Socket::Socket(SOCKET sockfd) :
	sockfd_(sockfd)
{

}

Socket::~Socket()
{
	if (sockfd_ > 0)
	{
		socketops::Close(sockfd_);
		sockfd_ = -1;
	}
}

SOCKET Socket::GetSockFd() const
{
	return sockfd_;
}

void Socket::SetSockfd(SOCKET sockfd)
{
	sockfd_ = sockfd;
}

void Socket::SetReusePort()
{
#ifdef _WIN32
#else
	socketops::SetReusePort(sockfd_);
#endif
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

SOCKET Socket::Accept(InetAddress* address)
{

	return socketops::Accept(sockfd_, address->GetSockAddr());
}

ssize_t Socket::Send(const char* data, size_t length)
{
	return socketops::Send(sockfd_, data, length);
}

void Socket::ShutdownWrite()
{
	socketops::ShutdownWrite(sockfd_);
}

