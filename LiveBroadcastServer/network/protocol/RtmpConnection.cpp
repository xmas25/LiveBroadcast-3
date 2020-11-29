//
// Created by rjd67 on 2020/11/29.
//

#include "network/protocol/RtmpConnection.h"
#include "utils/Logger.h"

RtmpConnection::RtmpConnection(const TcpConnectionPtr& connection_ptr) :
		connection_ptr_(connection_ptr),
		rtmp_manager_(),
		flv_manager_(rtmp_manager_.GetFlvManager()),
		last_write_size_(0),
		header_buffer_(),
		flv_tag_vector_(flv_manager_->GetFlvTags())
{

}

RtmpConnection::ShakeHandResult RtmpConnection::ShakeHand(Buffer* buffer)
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
				return RtmpConnection::SHAKE_SUCCESS;
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

ssize_t RtmpConnection::WriteToFile(File* file_write)
{
	LOG_INFO("connection: %s start write data to file", connection_ptr_->GetConnectionName().c_str());

	/**
	 * 写文件头 包含header, 脚本Tag, 第一个音频和第一个视频Tag
	 */
	FlvManager* flv_manager = rtmp_manager_.GetFlvManager();

	Buffer buffer;
	flv_manager->EncodeHeadersToBuffer(&buffer);
	file_write->Write(&buffer);


	/**
	 * 写文件体
	 */
	std::vector<FlvTag*>* flv_tags = flv_manager->GetFlvTags();

	ssize_t write_bytes;
	size_t sum_write_bytes = buffer.GetSumWrite();
	for (FlvTag* tag : *flv_tags)
	{
		write_bytes = file_write->Write(tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
		sum_write_bytes += write_bytes;

		write_bytes = file_write->Write(tag->GetBody());
		sum_write_bytes += write_bytes;
	}

	LOG_INFO("connection: %s write %zu bytes to %s success",
			connection_ptr_->GetConnectionName().c_str(),
			sum_write_bytes,
			file_write->GetPath().c_str());

	return sum_write_bytes;
}

ssize_t RtmpConnection::ParseData(Buffer* buffer)
{
	ssize_t result = rtmp_manager_.ParseData(buffer);
	DebugParseSize(10 * 1000 * 1000);

	return result;
}

void RtmpConnection::DebugParseSize(size_t division)
{
	size_t read_m = rtmp_manager_.GetParsedLength() / division;
	if (read_m != last_write_size_)
	{
		last_write_size_ = read_m;
		LOG_INFO("connection: %s, sum write %zu bytes", connection_ptr_->GetConnectionName().c_str(),
				rtmp_manager_.GetParsedLength());
	}
}

const Buffer* RtmpConnection::GetHeaderDataBuffer()
{
	if (header_buffer_.ReadableLength() == 0)
	{
		flv_manager_->EncodeHeadersToBuffer(&header_buffer_);
	}
	return &header_buffer_;
}

void RtmpConnection::OnConnectionShakeHand(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	RtmpConnection::ShakeHandResult result = ShakeHand(buffer);
	switch (result)
	{
		case RtmpConnection::SHAKE_SUCCESS:
		{
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
		case RtmpConnection::SHAKE_FAILED:
		{
			LOG_WARN("connection: %s shake hand failed",
					connection_ptr->GetConnectionName().c_str());
			connection_ptr->Shutdown();
			/**
			 * 出错时返回
			 */
			return;
		}
		case RtmpConnection::SHAKE_DATA_NOT_ENOUGH:
			/**
			 * 数据不足时返回
			 */
			return;
	}
}

void RtmpConnection::OnBodyData(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	ParseData(buffer);
}

const std::vector<FlvTag*>* RtmpConnection::GetFlvTagVector() const
{
	return flv_tag_vector_;
}
