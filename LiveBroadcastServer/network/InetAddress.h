#ifndef NETWORK_INETADDRESS
#define NETWORK_INETADDRESS

#include <cstdint>
#include <arpa/inet.h>
#include <string>

class InetAddress
{
public:
	InetAddress();
	InetAddress(uint16_t port, bool ipv6);

	std::string ToIpPort();

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
