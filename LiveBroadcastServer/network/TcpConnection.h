#ifndef NETWORK_TCPCONNECTION_H
#define NETWORK_TCPCONNECTION_H

#include <string>
#include "network/Socket.h"
#include "network/Channel.h"
#include "network/Callback.h"
#include "utils/Buffer.h"
#include "utils/Timestamp.h"

/**
 * Tcp连接类 包装Tcp连接
 * 其中含有Tcp连接的核心Socket以及地址
 * 与EventLoop交互的Channel
 * */
class EventLoop;
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
	/**
	 * 创建新的Tcp连接
	 * @param loop 所属的循环
	 * @param connection_name 连接名称
	 * @param sockfd 连接的文件描述符
	 * @param address 链接地址
	 */
	TcpConnection(EventLoop* loop, const std::string& connection_name, SOCKET sockfd, const InetAddress& address);
	~TcpConnection();
	/**
	 * 建立连接
	 */
	void Established();

	/**
	 * 关闭连接
	 */
	void CloseConnection();
	/**
	 * 设置新消息回调 当次Tcp连接有新的数据到达时 调用此回调
	 * @param callback
	 */
	void SetNewMessageCallback(const NewMessageCallback& callback);

	/**
	 * 设置连接信息回调函数 建立新的连接和断开连接均调用
	 * @param callback
	 */
	void SetConnectionCallback(const ConnectionCallback& callback);
	void SetConnectionCloseCallback(const ConnectionCallback& callback);

	const std::string& GetConnectionName() const;

	SOCKET GetSockfd() const;

	bool Connected() const;

	ssize_t Send(const char* data, size_t length);

	ssize_t Send(const uint8_t * data, size_t length);
private:
	enum Status {DISCONNECTING, DISCONNECTED, CONNECTING, CONNECTED};

	Status connection_status_;

	EventLoop* loop_;

	std::string connection_name_;

	Socket sockfd_;
	Channel channel_;

	InetAddress address_;

	ConnectionCallback connection_callback_;
	ConnectionCallback connection_close_callback_;
	NewMessageCallback newmessage_callback_;

	Buffer recv_buffer_;

	void OnReadable();
};

#endif // !NETWORK_TCPCONNECTION_H
