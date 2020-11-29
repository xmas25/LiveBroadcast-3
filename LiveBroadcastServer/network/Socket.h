#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H

#include <string>
#include <cassert>

#include "network/PlatformNetwork.h"
#include "network/InetAddress.h"

class Socket
{
public:
	Socket(SOCKET sockfd);
	~Socket();

	SOCKET GetSockFd() const;
	void SetSockfd(SOCKET sockfd);

	void SetReusePort();

	void SetReuseAddr();

	void Bind(const InetAddress& address);

	void Listen();

	SOCKET Accept(InetAddress* address);

	ssize_t Send(const char* data, size_t length);

	void ShutdownWrite();
private:

	SOCKET sockfd_;
};

#endif