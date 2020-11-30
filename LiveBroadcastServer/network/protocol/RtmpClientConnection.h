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
	FlvTagBuffer(const FlvTagPtr& flv_tag);

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

	void Send(const Buffer* buffer);

	void AddNewTag(const FlvTagBufferPtr& tag_buffer_ptr);
private:

	TcpConnectionPtr connection_ptr_;
};


#endif //LIVEBROADCASTSERVER_RTMPCLIENTCONNECTION_H
