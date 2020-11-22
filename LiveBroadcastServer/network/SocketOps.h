#ifndef NETWORK_SOCKETOPS
#define NETWORK_SOCKETOPS

#include "base/Platform.h"

namespace socketops
{
	SOCKET Socket(int family);

	int Htons(int port);

	void Bind(SOCKET sockfd, const sockaddr& address);

	void Listen(SOCKET sockfd);

	void SetReusePort(SOCKET sockfd);

	void SetReuseAddr(SOCKET sockfd);

	SOCKET Accept(SOCKET sockfd, struct sockaddr* address);
}

#endif // !NETWORK_SOCKETOPS