#include <sys/socket.h>
#include <netinet/in.h>
#include <cassert>
#include "network/SocketOps.h"

int socketops::Socket(int family)
{
	return ::socket(family, SOCK_STREAM, 0);
}

int socketops::Htons(int port)
{
	return ::htons(port);
}

void socketops::Bind(int sockfd, const sockaddr& address)
{
	int ret = ::bind(sockfd, &address, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
	assert(ret != -1);
}

void socketops::Listen(int sockfd)
{
	::listen(sockfd, 5);
}

void socketops::SetReusePort(int sockfd)
{
	int on = 1;
	int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &on,
		static_cast<socklen_t>(sizeof on));
	assert(ret != -1);
}

void socketops::SetReuseAddr(int sockfd)
{
	int on = 1;
	int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on,
		static_cast<socklen_t>(sizeof on));
	assert(ret != -1);
}

int socketops::Accept(int sockfd, struct sockaddr* address)
{
	socklen_t len = static_cast<socklen_t>(sizeof(*address));
	int fd = accept(sockfd, address, &len);

	assert(fd >= 0);

	return fd;
}
