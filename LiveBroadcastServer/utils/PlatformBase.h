//
// Created by rjd67 on 2020/11/21.
//

#ifndef NETWORK_PLATFORM_H
#define NETWORK_PLATFORM_H

#ifdef _WIN32
#define CRLF "\r\n"
#define DELIMITER '\\'

#include <winsock2.h>

#else
#define CRLF "\n"
#define DELIMITER '/'

#include <sys/socket.h>
#define SOCKET int
#endif

#endif //NETWORK_PLATFORM_H
