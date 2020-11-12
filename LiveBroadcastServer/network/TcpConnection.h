#ifndef NETWORK_TCPCONNECTION_H
#define NETWORK_TCPCONNECTION_H

#include <string>
#include "network/Socket.h"

class TcpConnection
{
public:



private:
	std::string connection_name_;

	Socket socket_;
};

#endif // !NETWORK_TCPCONNECTION_H
