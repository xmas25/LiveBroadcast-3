#ifndef NETWORK_ACCEPTOR_H
#define NETWORK_ACCEPTOR_H

#include "network/Socket.h"
#include "network/Channel.h"
#include "network/Callback.h"

typedef std::function<void(int, const InetAddress&)> NewConnectionCallback;

class EventLoop;
class Acceptor
{
public:
	Acceptor(EventLoop* loop, const InetAddress& address);
	~Acceptor();

	int Accept(InetAddress* address);

	void Listen();

	void SetNewConnectionCallback(const NewConnectionCallback& cb);

private:
	EventLoop* loop_;

	Socket listenfd_;

	Channel channel_;

	NewConnectionCallback newconnection_callback_;

	void OnReadable();
};

#endif // !NETWORK_ACCEPTOR_H
