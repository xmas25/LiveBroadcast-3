#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H

#include <string>
#include <cassert>

#include "base/Platform.h"
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
private:

	SOCKET sockfd_;
};

#endif