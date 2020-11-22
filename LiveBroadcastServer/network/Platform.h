//
// Created by rjd67 on 2020/11/21.
//

#ifndef NETWORK_PLATFORM_H
#define NETWORK_PLATFORM_H

enum EVENTS
{
	XEPOLLIN = 0x001,
#define XEPOLLIN XEPOLLIN
	XEPOLLPRI = 0x002,
#define XEPOLLPRI XEPOLLPRI
	XEPOLLOUT = 0x004,
#define XEPOLLOUT XEPOLLOUT
	XEPOLLMSG = 0x400,
#define XEPOLLMSG XEPOLLMSG
	XEPOLLERR = 0x008,
#define XEPOLLERR XEPOLLERR
	XEPOLLHUP = 0x010,
#define XEPOLLHUP XEPOLLHUP
	XEPOLLRDHUP = 0x2000,
#define XEPOLLRDHUP XEPOLLRDHUP
	XEPOLLWAKEUP = 1u << 29,
#define XEPOLLWAKEUP XEPOLLWAKEUP
	XEPOLLONESHOT = 1u << 30,
#define XEPOLLONESHOT XEPOLLONESHOT
	XEPOLLET = 1u << 31
#define XEPOLLET XEPOLLET
};

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <Windows.h>

class NetworkInitializer
{
public:
	NetworkInitializer();
	~NetworkInitializer();
};

std::string GetLastErrorAsString();

int write(SOCKET s, const uint8_t *buf, int len);

int read(SOCKET s, char *buf, int len);

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SOCKET int
#endif

#endif //NETWORK_PLATFORM_H
