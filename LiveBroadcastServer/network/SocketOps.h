#ifndef NETWORK_SOCKETOPS
#define NETWORK_SOCKETOPS

#include "network/PlatformNetwork.h"

namespace socketops
{
	SOCKET CreateDefaultSocket(int family);

	SOCKET CreateNonblockSocket(int family);

	int Htons(int port);

	void Bind(SOCKET sockfd, const sockaddr& address);

	int Connect(SOCKET sockfd, const sockaddr& address);

	void Listen(SOCKET sockfd);

	void SetReusePort(SOCKET sockfd);

	void SetReuseAddr(SOCKET sockfd);

	SOCKET Accept(SOCKET sockfd, struct sockaddr* address);

	ssize_t Send(SOCKET sockfd, const char* data, size_t length);

	void Close(SOCKET sockfd);

	void ShutdownWrite(SOCKET sockfd);

	sockaddr_in6 GetPeerAddr(SOCKET sockfd);
}

#endif // !NETWORK_SOCKETOPS