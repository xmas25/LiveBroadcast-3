#include <cassert>
#include "network/SocketOps.h"

SOCKET socketops::Socket(int family)
{
	return ::socket(family, SOCK_STREAM, 0);
}

int socketops::Htons(int port)
{
	return ::htons(port);
}

void socketops::Bind(SOCKET sockfd, const sockaddr& address)
{
	int ret = ::bind(sockfd, &address, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
	assert(ret != -1);
}

void socketops::Listen(SOCKET sockfd)
{
	::listen(sockfd, 5);
}

void socketops::SetReusePort(SOCKET sockfd)
{
#ifdef _WIN32
#else
	char on = 1;
	int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &on,
		static_cast<socklen_t>(sizeof on));
	assert(ret != -1);
#endif
}

void socketops::SetReuseAddr(SOCKET sockfd)
{
	char on = 1;
	int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on,
		static_cast<socklen_t>(sizeof on));
	assert(ret != -1);
}

SOCKET socketops::Accept(SOCKET sockfd, struct sockaddr* address)
{

	socklen_t len = INET6_ADDRSTRLEN;
	SOCKET fd = accept(sockfd, address, &len);

#ifdef _WIN32
	assert(fd != INVALID_SOCKET);
#else
	assert(fd >= 0);
#endif

	return fd;
}
