//
// Created by rjd67 on 2020/11/30.
//

#ifndef LIVEBROADCASTSERVER_RTMPCLIENTCONNECTION_H
#define LIVEBROADCASTSERVER_RTMPCLIENTCONNECTION_H

#include "network/TcpConnection.h"
#include "utils/codec/FlvCodec.h"

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

	void SendHeader(const Buffer* buffer);

	void AddNewTag(const FlvTagBufferPtr& tag_buffer_ptr);

	void SetCloseConnectionCallback(const ConnectionCallback& callback);
private:

	TcpConnectionPtr connection_ptr_;
	ConnectionCallback close_connection_callback_;

	void OnConnection(const TcpConnectionPtr& connection_ptr);
};


#endif //LIVEBROADCASTSERVER_RTMPCLIENTCONNECTION_H
