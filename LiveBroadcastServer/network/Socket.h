#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H

#include <string>
#include <cassert>

#include "network/InetAddress.h"

class Socket
{
public:
	Socket(int sockfd);
	~Socket();

	int GetSockFd() const;
	void SetSockfd(int sockfd);

	void SetReusePort();

	void SetReuseAddr();

	void Bind(const InetAddress& address);

	void Listen();

	int Accept(InetAddress* address);
private:

	int sockfd_;
};

#endif