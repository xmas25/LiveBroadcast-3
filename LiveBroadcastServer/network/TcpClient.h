//
// Created by rjd67 on 2021/1/14.
//

#ifndef LIVEBROADCASTSERVER_TCPCLIENT_H
#define LIVEBROADCASTSERVER_TCPCLIENT_H

#include "network/Connector.h"
/**
 * TcpClient 管理客户端连接
 */
class TcpClient
{
public:
	/**
	 * 创建TcpClient
	 * @param loop 所从属的EventLoop
	 * @param server_addr 服务器地址
	 * @param client_name TcpClient名称
	 */
	TcpClient(EventLoop* loop, const InetAddress& server_addr, const std::string& client_name);
	~TcpClient();

	/**
	 * 设置可读回调
	 * @param callback
	 */
	void SetNewMessageCallback(const NewMessageCallback& callback);

	/**
	 * 设置连接结束回调
	 * @param callback
	 */
	void SetConnectCallback(const ConnectionCallback& callback);

	void SetWriteCompleteCallback(const ConnectionCallback& callback);

	/**
	 * 尝试连接
	 */
	void Connect();

	/**
	 * 关闭连接
	 */
	void DisConnect();

	/**
	 * 设置断线重连状态
	 * @param open true 开启断线重连 false 关闭
	 */
	void SetReConnect(bool open);
private:

	EventLoop* loop_;
	InetAddress server_addr_;
	std::string client_name_;

	NewMessageCallback newmessage_callback_;
	ConnectionCallback connection_callback_;
	ConnectionCallback write_complete_callback_;

	/** 管理的连接*/
	TcpConnectionPtr connection_ptr_;

	Connector connector_;

	int connection_id_;

	bool retry_;
	bool connect_;

	void OnNewConnection(SOCKET sockfd);

	void RemoveConnection(const TcpConnectionPtr& connection);
};


#endif //LIVEBROADCASTSERVER_TCPCLIENT_H
