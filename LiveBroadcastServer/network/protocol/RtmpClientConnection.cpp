//
// Created by rjd67 on 2020/11/30.
//

#include "network/protocol/RtmpClientConnection.h"
#include "utils/Format.h"

RtmpClientConnection::RtmpClientConnection(const TcpConnectionPtr& connection_ptr):
	connection_ptr_(connection_ptr)
{
	connection_ptr_->SetConnectionCallback(
			[this](auto&& PH1){OnConnection(PH1);});
}

RtmpClientConnection::~RtmpClientConnection()
{

}

std::string RtmpClientConnection::GetConnectionName() const
{
	return connection_ptr_->GetConnectionName();
}

void RtmpClientConnection::SendHeader(const Buffer* buffer)
{
	connection_ptr_->Send(buffer);
}

void RtmpClientConnection::AddNewTag(const FlvTagBufferPtr& tag_buffer_ptr)
{
	connection_ptr_->Send(tag_buffer_ptr->GetBuffer());
}

void RtmpClientConnection::SetCloseConnectionCallback(const ConnectionCallback& callback)
{
	close_connection_callback_ = callback;
}

void RtmpClientConnection::OnConnection(const TcpConnectionPtr& connection_ptr)
{
	if (!connection_ptr->Connected())
	{
		assert(connection_ptr->GetConnectionName() == connection_ptr_->GetConnectionName());
		if (close_connection_callback_)
		{
			close_connection_callback_(connection_ptr);
		}
	}
}

FlvTagBuffer::FlvTagBuffer(const FlvTagPtr& flv_tag) :
		buffer_size_(FlvTag::FLV_TAG_HEADER_LENGTH + flv_tag->GetBody()->ReadableLength()),
		buffer_(buffer_size_)
{
	std::string length_rn = Format::ToHexStringWithCrlf(buffer_size_);
	buffer_.AppendData(length_rn);
	buffer_.AppendData(flv_tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
	buffer_.AppendData(flv_tag->GetBody());
	buffer_.AppendData("\r\n");

	assert(buffer_.ReadableLength() - length_rn.size() - 2 == buffer_size_);
}

const Buffer* FlvTagBuffer::GetBuffer() const
{
	return &buffer_;
}
