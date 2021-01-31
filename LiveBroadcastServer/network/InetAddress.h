#ifndef NETWORK_INETADDRESS
#define NETWORK_INETADDRESS

#include <cstdint>
#include <string>
#include "network/PlatformNetwork.h"

class InetAddress
{
public:
	InetAddress();
	InetAddress(sockaddr_in6 addr);
	explicit InetAddress(uint16_t port, bool ipv6 = false);

	InetAddress(const std::string& addr, uint16_t port, bool ipv6 = false);

	std::string ToIpPort() const;

	struct sockaddr* GetSockAddr();

	const struct sockaddr* GetSockAddr() const;

	int GetFamily() const;
private:
	union
	{
		sockaddr_in addr_ipv4;
		sockaddr_in6 addr_ipv6;
	};

	int family_;
};

#endif // !NETWORK_INETADDRESS
