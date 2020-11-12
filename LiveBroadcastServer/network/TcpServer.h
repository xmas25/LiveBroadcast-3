#ifndef NETWORK_OBSSERVER_H
#define NETWORK_OBSSERVER_H

#include <cstdint>
#include <string>
#include <sys/socket.h>

#include "network/Socket.h"
#include "network/Acceptor.h"
#include "network/EventLoop.h"

class TcpServer
{
public:
	TcpServer(EventLoop* loop, const InetAddress& address);
	~TcpServer();

	void Start();

	void SetNewConnectionCallback(const NewConnectionCallback& cb);

private:
	EventLoop* loop_;

	Acceptor acceptor_;
};


#endif // !NETWORK_OBSSERVER_H

