#ifndef NETWORK_SOCKETOPS
#define NETWORK_SOCKETOPS

#include "network/PlatformNetwork.h"

namespace socketops
{
	SOCKET Socket(int family);

	int Htons(int port);

	void Bind(SOCKET sockfd, const sockaddr& address);

	void Listen(SOCKET sockfd);

	void SetReusePort(SOCKET sockfd);

	void SetReuseAddr(SOCKET sockfd);

	SOCKET Accept(SOCKET sockfd, struct sockaddr* address);

	ssize_t Send(SOCKET sockfd, const char* data, size_t length);
}

#endif // !NETWORK_SOCKETOPS