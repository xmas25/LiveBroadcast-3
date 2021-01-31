//
// Created by rjd67 on 2020/11/25.
//

#ifndef LIVEBROADCASTSERVER_PLATFORMNETWORK_H
#define LIVEBROADCASTSERVER_PLATFORMNETWORK_H

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

#include <string>
std::string GetLastErrorAsString();

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#define SOCK_NONBLOCK 0
#define SOCK_CLOEXEC 0

class NetworkInitializer
{
public:
	NetworkInitializer();
	~NetworkInitializer();
};

int write(SOCKET s, const uint8_t *buf, int len);

int read(SOCKET s, char *buf, int len);

#define SHUT_WR SD_SEND

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

#define SOCKET int
#endif

#endif //LIVEBROADCASTSERVER_PLATFORMNETWORK_H
