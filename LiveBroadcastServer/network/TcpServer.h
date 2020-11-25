#ifndef NETWORK_OBSSERVER_H
#define NETWORK_OBSSERVER_H

#include <cstdint>
#include <string>
#include <map>

#include "network/TcpConnection.h"
#include "network/Acceptor.h"

class TcpServer
{
public:

	typedef std::map<std::string, TcpConnectionPtr> TcpConnectionMap;

	TcpServer(EventLoop* loop, const std::string& server_name, const InetAddress& address);
	~TcpServer();

	void Start();

	void SetNewConnectionCallback(const NewConnectionCallback& cb);

	void SetNewMessageCallback(const NewMessageCallback& callback);

private:
	EventLoop* loop_;

	std::string server_name_;

	Acceptor acceptor_;

	NewConnectionCallback newconnection_callback_;
	NewMessageCallback newmessage_callback_;

	TcpConnectionMap connection_map_;

	int connection_id_;

	void OnNewConnection(SOCKET sockfd, const InetAddress& address);

	void OnCloseConnection(const TcpConnectionPtr& connection_ptr);
};


#endif // !NETWORK_OBSSERVER_H

