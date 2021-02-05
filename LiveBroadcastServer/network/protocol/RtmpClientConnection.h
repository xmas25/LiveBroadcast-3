//
// Created by rjd67 on 2020/11/30.
//

#ifndef LIVEBROADCASTSERVER_RTMPCLIENTCONNECTION_H
#define LIVEBROADCASTSERVER_RTMPCLIENTCONNECTION_H

#include "network/TcpConnection.h"
#include "utils/codec/FlvCodec.h"

/**
 * FlvTag包装器 将单个FlvTag进行包装, 然后通过指针在各个
 * 客户端中进行共享
 *
 * 由于会使用shared_ptr包装指针不需要进行delete 自动丢弃
 */
class FlvTagBuffer
{
public:
	explicit FlvTagBuffer(const FlvTagPtr& flv_tag);

	const Buffer* GetBuffer() const;
private:
	size_t buffer_size_;
	Buffer buffer_;
};
typedef std::shared_ptr<FlvTagBuffer> FlvTagBufferPtr;

class RtmpClientConnection
{
public:
	explicit RtmpClientConnection(const TcpConnectionPtr& connection_ptr);
	~RtmpClientConnection();

	std::string GetConnectionName() const;


	/**
	 * 在连接刚建立时 需要发送一次包含元数据的头部
	 * @param buffer
	 */
	void SendHeader(const Buffer* buffer);

	/**
	 * 向客户端添加新的 flv_tag 并进行发送
	 * @param tag_buffer_ptr 新的flv_tag指针
	 */
	void AddNewTag(const FlvTagBufferPtr& tag_buffer_ptr);

	void SetCloseConnectionCallback(const ConnectionCallback& callback);
private:

	TcpConnectionPtr connection_ptr_;
	ConnectionCallback close_connection_callback_;

	void OnConnection(const TcpConnectionPtr& connection_ptr);
};


#endif //LIVEBROADCASTSERVER_RTMPCLIENTCONNECTION_H
