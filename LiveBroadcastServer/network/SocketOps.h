#ifndef NETWORK_SOCKETOPS
#define NETWORK_SOCKETOPS

#include <arpa/inet.h>


namespace socketops
{
	int Socket(int family);

	int Htons(int port);

	void Bind(int sockfd, const sockaddr& address);

	void Listen(int sockfd);

	void SetReusePort(int sockfd);

	void SetReuseAddr(int sockfd);

	int Accept(int sockfd, struct sockaddr* address);
}

#endif // !NETWORK_SOCKETOPS