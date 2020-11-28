//
// Created by rjd67 on 2020/11/25.
//

#ifndef LIVEBROADCASTSERVER_LOGGER_H
#define LIVEBROADCASTSERVER_LOGGER_H

#include <cstdio>
#include <cstring>
#include "utils/PlatformBase.h"


#define __FILENAME__ (strrchr(__FILE__, DELIMITER) ? strrchr(__FILE__, DELIMITER) + 1 : __FILE__)



#define LOG_INFO(...) LOG("INFO ", __VA_ARGS__)

#define LOG_WARN(...) LOG("WARN ", __VA_ARGS__)

#define LOG_ERROR(...) LOG("ERROR", __VA_ARGS__)

#define LOG(level, ...)	\
{	\
	printf("[%s][ %s:%d ]: ", level, __FILENAME__, __LINE__);	\
	printf(__VA_ARGS__);	\
	printf(CRLF);          \
	fflush(stdout);                  \
}

class Logger
{

};


#endif //LIVEBROADCASTSERVER_LOGGER_H
