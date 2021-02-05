#include <cstring>

#include "network/InetAddress.h"
#include "network/SocketOps.h"

InetAddress::InetAddress()
{
}

InetAddress::InetAddress(sockaddr_in6 addr)
{
	family_ = AF_INET6;
	addr_ipv6 = addr;
}

InetAddress::InetAddress(uint16_t port, bool ipv6) :
	family_(ipv6 ? AF_INET6 : AF_INET)
{
	if (ipv6)
	{
		memset(&addr_ipv6, 0, sizeof addr_ipv6);
		addr_ipv6.sin6_port = socketops::Htons(port);
		addr_ipv6.sin6_family = PF_INET6;
	}
	else
	{
		memset(&addr_ipv4, 0, sizeof addr_ipv4);
		addr_ipv4.sin_port = socketops::Htons(port);
		addr_ipv4.sin_family = PF_INET;
	}
}


InetAddress::InetAddress(const std::string& addr, uint16_t port, bool ipv6) :
	InetAddress(port, ipv6)
{
	if (ipv6)
	{
		socketops::NameToAddr6(addr, &addr_ipv6.sin6_addr);
	}
	else
	{
		socketops::NameToAddr4(addr, &addr_ipv4.sin_addr);
	}
}

std::string InetAddress::ToIpPort() const
{
	char buffer[64]{};

	const struct sockaddr* addr = (const struct sockaddr*)&addr_ipv4;

	if (addr->sa_family == AF_INET)
	{
#ifndef _WIN32
		inet_ntop(AF_INET, &addr_ipv4.sin_addr, buffer, sizeof buffer);
#endif
	}
	else if (addr->sa_family == AF_INET6)
	{
#ifndef _WIN32
		inet_ntop(AF_INET6, &addr_ipv6.sin6_addr, buffer, sizeof buffer);
#endif
	}
	else
	{
		return std::string();
	}
	size_t ip_len = strlen(buffer);
	snprintf(buffer + ip_len, sizeof buffer - ip_len, ":%hd", htons(addr_ipv4.sin_port));

	return std::string(buffer);
}

sockaddr* InetAddress::GetSockAddr()
{
	return reinterpret_cast<sockaddr*>(&addr_ipv6);
}

const sockaddr* InetAddress::GetSockAddr() const
{
	return reinterpret_cast<const sockaddr*>(&addr_ipv6);
}

int InetAddress::GetFamily() const
{
	return family_;
}
