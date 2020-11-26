#ifndef NETWORK_ACCEPTOR_H
#define NETWORK_ACCEPTOR_H

#include "network/Socket.h"
#include "network/Channel.h"

class EventLoop;
class Acceptor
{
public:
	typedef std::function<void(SOCKET, const InetAddress&)> NewConnectionCallback;

	Acceptor(EventLoop* loop, const std::string& server_name, const InetAddress& address);
	~Acceptor();

	SOCKET Accept(InetAddress* address);

	void Listen();

	void SetNewConnectionCallback(const NewConnectionCallback& cb);

private:
	EventLoop* loop_;

	std::string name_;

	Socket listenfd_;

	Channel channel_;

	NewConnectionCallback newconnection_callback_;

	void OnReadable();
};

#endif // !NETWORK_ACCEPTOR_H
