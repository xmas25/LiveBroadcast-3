#include <cassert>
#include <unistd.h>
#include <netdb.h>
#include "network/SocketOps.h"
#include "utils/Logger.h"

SOCKET socketops::CreateDefaultSocket(int family)
{
	return ::socket(family, SOCK_STREAM | SOCK_CLOEXEC, 0);
}

SOCKET socketops::CreateNonblockSocket(int family)
{
	return ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
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

int socketops::Connect(SOCKET sockfd, const sockaddr& address)
{
	return ::connect(sockfd, &address, static_cast<socklen_t>(sizeof address));
}

void socketops::Listen(SOCKET sockfd)
{
	::listen(sockfd, 5);
}

void socketops::SetReusePort(SOCKET sockfd)
{
#ifndef _WIN32
	int on = 1;
	int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &on,
		static_cast<socklen_t>(sizeof on));
	assert(ret != -1);
#endif
}

void socketops::SetReuseAddr(SOCKET sockfd)
{
#ifdef _WIN32
	char on = 1;
#else
	int on = 1;
#endif
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

void socketops::Close(SOCKET sockfd)
{
	::close(sockfd);
}

ssize_t socketops::Send(SOCKET sockfd, const char* data, size_t length)
{
	return send(sockfd, data, length, 0);
}

void socketops::ShutdownWrite(SOCKET sockfd)
{
	::shutdown(sockfd, SHUT_WR);
}

sockaddr_in6 socketops::GetPeerAddr(SOCKET sockfd)
{
	struct sockaddr_in6 peer_addr;
	socklen_t len = static_cast<socklen_t>(sizeof peer_addr);
	getpeername(sockfd, (sockaddr*)&peer_addr, &len);
	return peer_addr;
}

int socketops::InetPton(int af, const char* from, void* to)
{
	return ::inet_pton(af, from, to);
}

bool socketops::NameToAddr4(const std::string& name, in_addr* addr)
{
	if (InetPton(AF_INET, name.c_str(), addr) == 0)
	{
		hostent* host = gethostbyname(name.c_str());
		if (host)
		{
			addr->s_addr = *((uint32_t*)host->h_addr);
		}
		else
		{
			LOG_ERROR("parse name: %s error", name.c_str());
			return false;
		}
	}
	return true;
}

void socketops::NameToAddr6(const std::string& name, in6_addr* addr)
{

}
