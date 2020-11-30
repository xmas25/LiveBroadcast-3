//
// Created by rjd67 on 2020/11/29.
//

#include "network/protocol/RtmpServerConnection.h"
#include "utils/Logger.h"
#include "utils/Format.h"

RtmpServerConnection::RtmpServerConnection(const TcpConnectionPtr& connection_ptr) :
		connection_ptr_(connection_ptr),
		rtmp_manager_(),
		flv_manager_(rtmp_manager_.GetFlvManager()),
		last_write_size_(0),
		header_buffer_()
{

}

RtmpServerConnection::ShakeHandResult RtmpServerConnection::ShakeHand(Buffer* buffer)
{
	while (true)
	{
		RtmpManager::ShakeHandPackType status = rtmp_manager_.ParseShakeHand(buffer);
		switch (status)
		{
			case RtmpManager::SHAKE_RTMP_C01:
				connection_ptr_->Send(RTMP_SERVER_S01, sizeof RTMP_SERVER_S01);
				break;
			case RtmpManager::SHAKE_RTMP_C2:
				connection_ptr_->Send(RTMP_SERVER_S2, sizeof RTMP_SERVER_S2);
				break;
			case RtmpManager::SHAKE_RTMP_SET_CHUNK_SIZE:
				break;
			case RtmpManager::SHAKE_RTMP_CONNECT:
			{
				connection_ptr_->Send(RTMP_SERVER_ACKNOWLEDGE_SIZE, sizeof RTMP_SERVER_ACKNOWLEDGE_SIZE);
				connection_ptr_->Send(RTMP_SERVER_PEER_BANDWIDTH, sizeof RTMP_SERVER_PEER_BANDWIDTH);
				connection_ptr_->Send(RTMP_SERVER_SET_CHUNK_SIZE, sizeof RTMP_SERVER_SET_CHUNK_SIZE);
				connection_ptr_->Send(RTMP_SERVER_CONNECT_RESULT, sizeof RTMP_SERVER_CONNECT_RESULT);
				break;
			}
			case RtmpManager::SHAKE_RTMP_RELEASE_STREAM:
				break;
			case RtmpManager::SHAKE_RTMP_FCPUBLISH:
				break;
			case RtmpManager::SHAKE_RTMP_CREATE_STREAM:
				connection_ptr_->Send(RTMP_SERVER_RESULT, sizeof RTMP_SERVER_RESULT);
				break;
			case RtmpManager::SHAKE_RTMP_PUBLISH:
				connection_ptr_->Send(RTMP_SERVER_START, sizeof RTMP_SERVER_START);
				break;
			case RtmpManager::SHAKE_SUCCESS:
			{
				return RtmpServerConnection::SHAKE_SUCCESS;
			}
			case RtmpManager::SHAKE_FAILED:
			{
				/**
				 * 出错时返回
				 */
				return SHAKE_FAILED;
			}
			case RtmpManager::SHAKE_DATA_NOT_ENOUGH:
				/**
				 * 数据不足时返回
				 */
				return SHAKE_DATA_NOT_ENOUGH;
		}
	}
}

ssize_t RtmpServerConnection::WriteToFile(File* file_write)
{
	LOG_INFO("connection: %s start write data to file", connection_ptr_->GetConnectionName().c_str());

	/**
	 * 写文件头 包含header, 脚本Tag, 第一个音频和第一个视频Tag
	 */
	FlvManager* flv_manager = rtmp_manager_.GetFlvManager();

	const Buffer* buffer = GetHeaderDataBuffer();
	file_write->Write(buffer);


	/**
	 * 写文件体
	 */
	std::vector<FlvTag*>* flv_tags = flv_manager->GetFlvTags();

	ssize_t write_bytes;
	size_t sum_write_bytes = buffer->ReadableLength();
	for (FlvTag* tag : *flv_tags)
	{
		write_bytes = file_write->Write(tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
		sum_write_bytes += write_bytes;

		write_bytes = file_write->Write(tag->GetBody());
		sum_write_bytes += write_bytes;
	}

	/**
	 * 文件尾部为最后一个tag的大小
	 */
	uint32_t previous_tag_size = (flv_tags->at(flv_tags->size() - 1))->GetCurrentTagSize();
	previous_tag_size = htonl(previous_tag_size);
	file_write->Write((char*)&previous_tag_size, sizeof previous_tag_size);

	LOG_INFO("connection: %s write %zu bytes to %s success",
			connection_ptr_->GetConnectionName().c_str(),
			sum_write_bytes,
			file_write->GetPath().c_str());

	return sum_write_bytes;
}

ssize_t RtmpServerConnection::ParseData(Buffer* buffer)
{
	ssize_t result = rtmp_manager_.ParseData(buffer);
	DebugParseSize(10 * 1000 * 1000);

	return result;
}

void RtmpServerConnection::DebugParseSize(size_t division)
{
	size_t read_m = rtmp_manager_.GetParsedLength() / division;
	if (read_m != last_write_size_)
	{
		last_write_size_ = read_m;
		LOG_INFO("connection: %s, sum write %zu bytes", connection_ptr_->GetConnectionName().c_str(),
				rtmp_manager_.GetParsedLength());
	}
}
std::string response_header = "HTTP/1.1 200 OK\r\n"
							  "Server: FISH_LIVE\r\n"
							  "Date: Sun, 29 Nov 2020 15:30:42 GMT\r\n"
							  "Content-Type: video/x-flv\r\n"
							  "Transfer-Encoding: chunked\r\n"
							  "Connection: keep-alive\r\n"
							  "Access-Control-Allow-Origin: *\r\n\r\n";

const Buffer* RtmpServerConnection::GetHeaderDataBuffer()
{
	if (header_buffer_.ReadableLength() == 0)
	{
		Buffer header_buffer_temp;
		flv_manager_->EncodeHeadersToBuffer(&header_buffer_temp);

		std::string length_rn = Format::ToHexStringWithCrlf(header_buffer_temp.ReadableLength());
		header_buffer_.AppendData(response_header);
		header_buffer_.AppendData(length_rn);
		header_buffer_.AppendData(&header_buffer_temp);
		header_buffer_.AppendData("\r\n");
	}
	return &header_buffer_;
}

void RtmpServerConnection::OnConnectionShakeHand(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	RtmpServerConnection::ShakeHandResult result = ShakeHand(buffer);
	switch (result)
	{
		case RtmpServerConnection::SHAKE_SUCCESS:
		{
			if (shake_hand_success_callback_)
			{
				shake_hand_success_callback_(this);
			}

			connection_ptr->SetNewMessageCallback(
					[this](auto&& PH1, auto&& PH2, auto&& PH3)
					{
						OnBodyData(PH1, PH2, PH3);
					}
					);
			LOG_INFO("connection: %s shake hand success",
					connection_ptr->GetConnectionName().c_str());
			/**
			 * 握手成功时返回
			 */
			return;
		}
		case RtmpServerConnection::SHAKE_FAILED:
		{
			LOG_WARN("connection: %s shake hand failed",
					connection_ptr->GetConnectionName().c_str());
			connection_ptr->Shutdown();
			/**
			 * 出错时返回
			 */
			return;
		}
		case RtmpServerConnection::SHAKE_DATA_NOT_ENOUGH:
			/**
			 * 数据不足时返回
			 */
			return;
	}
}

void RtmpServerConnection::OnBodyData(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	ParseData(buffer);
}

void RtmpServerConnection::AddClientConnection(
		const RtmpClientConnectionPtr& client_connection_ptr)
{
	client_connection_map_[client_connection_ptr->GetConnectionName()]
		= client_connection_ptr;

	SendHeaderToClientConnection(client_connection_ptr);
}

void RtmpServerConnection::SetShakeHandSuccessCallback(const ShakeHandSuccessCallback& callback)
{
	shake_hand_success_callback_ = callback;
}

void RtmpServerConnection::SendHeaderToClientConnection(
		const RtmpClientConnectionPtr& client_connection_ptr)
{
	const Buffer* header_buffer = GetHeaderDataBuffer();

	client_connection_ptr->Send(header_buffer);
}

void RtmpServerConnection::OnNewFlvTag(const FlvTagPtr& tag_ptr)
{
	FlvTagBufferPtr buffer_ptr = std::make_shared<FlvTagBuffer>(tag_ptr);

	for (auto& [connection_name, connection_ptr] : client_connection_map_)
	{
		connection_ptr->AddNewTag(buffer_ptr);
	}
}

