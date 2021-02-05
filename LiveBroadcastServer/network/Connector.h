//
// Created by rjd67 on 2021/1/14.
//

#ifndef LIVEBROADCASTSERVER_CONNECTOR_H
#define LIVEBROADCASTSERVER_CONNECTOR_H

#include "network/EventLoop.h"
#include "network/InetAddress.h"

/**
 * 连接创建者
 */
class Connector
{
public:

	typedef std::function<void(int)> ConnectCallback;

	/**
	 *
	 * @param loop 从属EventLoop
	 * @param server_addr 服务器地址
	 */
	Connector(EventLoop* loop, const InetAddress& server_addr);
	~Connector();

	/**
	 * 尝试连接 会进行可写测试 确认连接已经建立
	 *
	 * 可写测试成功回调为HandleWrite
	 */
	void Connect();

	/**
	 * 连接关闭
	 */
	void Disconnect();

	/**
	 * 设置connection_callback 在Connect调用连接操作后进行调用
	 * @param connection_callback
	 */
	void SetConnectCallback(const ConnectCallback& connection_callback);

	/**
	 * 进行重连
	 */
	void ReConnect();

private:

	enum Status{DISCONNECTED, CONNECTING, CONNECTED};

	EventLoop* loop_;
	InetAddress server_addr_;

	ConnectCallback connection_callback_;

	Status status_;

	/** 是否进行连接*/
	bool connect_;

	/** 用于设置可写回调验证链接是否真正建立*/
	std::unique_ptr<Channel> channel_ptr_;

	void Retry(SOCKET sockfd);

	/** 用于设置可写回调验证链接是否真正建立*/
	void Connecting(SOCKET sockfd);

	/**
	 * 主要目的重置Channel设置的可写回调事件
	 * @return Channel对应的文件描述符
	 */
	SOCKET RemoveAndResetChannel();

	/** 用于验证连接建立的可写回调*/
	void HandleWrite();
};


#endif //LIVEBROADCASTSERVER_CONNECTOR_H
