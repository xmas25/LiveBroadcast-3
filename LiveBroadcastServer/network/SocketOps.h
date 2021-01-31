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

	int InetPton(int af, const char* from, void* to);

	/**
	 * 将地址转换为网络地址
	 * @param name 域名或者ip地址
	 * @param addr 网络地址 ipv4
	 * @return 转换成功true
	 */
	bool NameToAddr4(const std::string& name, in_addr* addr);

	/**
	* 将地址转换为网络地址
	* @param name 域名或者ip地址
	* @param addr 网络地址 ipv6
	* @return 转换成功true
	*/
	void NameToAddr6(const std::string& name, in6_addr* addr);
}

#endif // !NETWORK_SOCKETOPS