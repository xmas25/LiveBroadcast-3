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
	rtmp_manager_.SetNewFlvTagCallback(
			[this](auto&& PH1){RtmpServerConnection::OnNewFlvTag(PH1);}
			);
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
			/** 解析完release包后 进行用户校验*/
			case RtmpManager::SHAKE_RTMP_RELEASE_STREAM:
			{
				return SHAKE_AUTHENTICATE;
			}
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

ssize_t RtmpServerConnection::ParseData(Buffer* buffer)
{
	ssize_t result = rtmp_manager_.ParseData(buffer);
	DebugParseSize(100 * 1000 * 1000);

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
		case RtmpServerConnection::SHAKE_AUTHENTICATE:
		{
			if (!Authenticate())
			{
				LOG_WARN("connection: %s authenticate failed",
						connection_ptr->GetConnectionName().c_str());
				connection_ptr->Shutdown();
				/**
				 * 校验出错时返回
				 */
				return;
			}
			return;
		}
		case RtmpServerConnection::SHAKE_SUCCESS:
		{
			LOG_INFO("connection: %s shake hand success",
					connection_ptr->GetConnectionName().c_str());
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

	LOG_INFO("server: %s, add a client: %s", connection_ptr_->GetConnectionName().c_str(),
			client_connection_ptr->GetConnectionName().c_str());

	client_connection_ptr->SetCloseConnectionCallback(
			[this](auto&& PH1){OnConnectionClose(PH1);});

	SendHeaderToClientConnection(client_connection_ptr);
}

void RtmpServerConnection::SetShakeHandSuccessCallback(const ShakeHandSuccessCallback& callback)
{
	shake_hand_success_callback_ = callback;
}

std::string RtmpServerConnection::GetRtmpUrl() const
{
	return rtmp_manager_.GetUrlFromConnectPack();
}

std::string RtmpServerConnection::GetRtmpPath() const
{
	return Format::GetPathFromUrl(GetRtmpUrl());
}

std::string RtmpServerConnection::GetConnectionName() const
{
	return connection_ptr_->GetConnectionName();
}

void RtmpServerConnection::SetAuthenticationCallback(const AuthenticationCallback& callback)
{
	authentication_callback_ = callback;
}

void RtmpServerConnection::SendHeaderToClientConnection(
		const RtmpClientConnectionPtr& client_connection_ptr)
{
	const Buffer* header_buffer = GetHeaderDataBuffer();

	client_connection_ptr->SendHeader(header_buffer);

	/**
	 * 头部之后第一个 Tag的PreviousTagSize 需要设置为 头部中最后一个Tag的CurrentSize
	 */
	last_flv_tag_ptr_->SetPreviousTagSize(GetLastHeaderTagCurrentSize());
	client_connection_ptr->AddNewTag(std::make_shared<FlvTagBuffer>(last_flv_tag_ptr_));
}

void RtmpServerConnection::OnNewFlvTag(const FlvTagPtr& tag_ptr)
{
	last_flv_tag_ptr_ = tag_ptr;
	FlvTagBufferPtr buffer_ptr = std::make_shared<FlvTagBuffer>(tag_ptr);

	for (auto& [connection_name, connection_ptr] : client_connection_map_)
	{
		connection_ptr->AddNewTag(buffer_ptr);
	}
}

uint32_t RtmpServerConnection::GetLastHeaderTagCurrentSize() const
{
	return flv_manager_->GetVideoAudioTags()[1].GetCurrentTagSize();
}

void RtmpServerConnection::OnConnectionClose(const TcpConnectionPtr& connection_ptr)
{
	client_connection_map_.erase(connection_ptr->GetConnectionName());
	LOG_INFO("client: %s, remove from server: %s", connection_ptr->GetConnectionName().c_str(),
			connection_ptr_->GetConnectionName().c_str());
}

bool RtmpServerConnection::Authenticate()
{
	std::string username = GetRtmpPath();
	std::string password = rtmp_manager_.GetPasswordFromReleasePack();

	if (authentication_callback_)
	{
		if (!authentication_callback_(username, password))
		{
			return false;
		}
	}

	return true;
}
