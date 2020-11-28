// LiveBroadcastServer.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <unistd.h>
#include <ctime>

#include "network/TcpServer.h"
#include "network/EventLoop.h"
#include "utils/codec/RtmpManager.h"
#include "utils/Logger.h"

#ifdef _WIN32
NetworkInitializer init;
#endif

std::string ROOT = R"(C:\Users\rjd67\Desktop\Server\)";
std::string FILE_PREFIX = ".flv";

ssize_t WriteToFile(FlvManager* flv_manager, File* file_write)
{
	/**
	 * 写文件头 包含header, 脚本Tag, 第一个音频和第一个视频Tag
	 */
	Buffer buffer;
	flv_manager->EncodeHeadersToBuffer(&buffer);
	file_write->Write(&buffer);


	/**
	 * 写文件体
	 */
	std::vector<FlvTag*>* flv_tags = flv_manager->GetFlvTags();

	ssize_t write_bytes;
	ssize_t sum_write_bytes = buffer.GetSumWrite();
	for (FlvTag* tag : *flv_tags)
	{
		write_bytes = file_write->Write(tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
		assert(write_bytes == FlvTag::FLV_TAG_HEADER_LENGTH);
		sum_write_bytes += write_bytes;

		const Buffer* body = tag->GetBody();
		write_bytes = file_write->Write(body);

		assert(write_bytes == static_cast<ssize_t>(body->ReadableLength()));
		sum_write_bytes += write_bytes;
	}
	
	return sum_write_bytes;
}

std::map<std::string, RtmpManager*> rtmp_manager_map;
std::map<std::string, size_t> last_write_m;

void OnConnection(const TcpConnectionPtr& connection_ptr)
{
	if (connection_ptr->Connected())
	{
		rtmp_manager_map[connection_ptr->GetConnectionName()] = new RtmpManager;
		last_write_m[connection_ptr->GetConnectionName()] = 0;
		
		LOG_INFO("connection: %s start parse", connection_ptr->GetConnectionName().c_str());
	}
	else
	{
		LOG_INFO("connection: %s start write data to file", connection_ptr->GetConnectionName().c_str());
		
		RtmpManager* rtmp_manager = rtmp_manager_map[connection_ptr->GetConnectionName()];

		time_t t = time(nullptr);
		File file_write(ROOT + std::to_string(t) + FILE_PREFIX, File::O_WRONLY);
		ssize_t sum_write_bytes = WriteToFile(rtmp_manager->GetFlvManager(), &file_write);
		
		LOG_INFO("connection: %s write %zob bytes to %s success",
				connection_ptr->GetConnectionName().c_str(),
				sum_write_bytes,
				file_write.GetPath().c_str());
	}
}

void OnNewMessage(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	RtmpManager* rtmp_manager = rtmp_manager_map[connection_ptr->GetConnectionName()];

	rtmp_manager->ParseData(buffer);

	size_t read_m = buffer->GetSumRead() / 1000000;
	if (read_m != last_write_m[connection_ptr->GetConnectionName()])
	{
		last_write_m[connection_ptr->GetConnectionName()] = read_m;
		LOG_INFO("connection: %s, sum write %zuMb", connection_ptr->GetConnectionName().c_str(),
				read_m);
	}
}

void OnConnectionShakeHand(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	RtmpManager* rtmp_manager = rtmp_manager_map[connection_ptr->GetConnectionName()];

	while (true)
	{
		RtmpManager::ShakeHandPackType status = rtmp_manager->ParseShakeHand(buffer);
		switch (status)
		{
			case RtmpManager::SHAKE_RTMP_C01:
				connection_ptr->Send(RTMP_SERVER_S01, sizeof RTMP_SERVER_S01);
				break;
			case RtmpManager::SHAKE_RTMP_C2:
				connection_ptr->Send(RTMP_SERVER_S2, sizeof RTMP_SERVER_S2);
				break;
			case RtmpManager::SHAKE_RTMP_SET_CHUNK_SIZE:
				break;
			case RtmpManager::SHAKE_RTMP_CONNECT:
			{
				connection_ptr->Send(RTMP_SERVER_ACKNOWLEDGE_SIZE, sizeof RTMP_SERVER_ACKNOWLEDGE_SIZE);
				connection_ptr->Send(RTMP_SERVER_PEER_BANDWIDTH, sizeof RTMP_SERVER_PEER_BANDWIDTH);
				connection_ptr->Send(RTMP_SERVER_SET_CHUNK_SIZE, sizeof RTMP_SERVER_SET_CHUNK_SIZE);
				connection_ptr->Send(RTMP_SERVER_CONNECT_RESULT, sizeof RTMP_SERVER_CONNECT_RESULT);
				break;
			}
			case RtmpManager::SHAKE_RTMP_RELEASE_STREAM:
				break;
			case RtmpManager::SHAKE_RTMP_FCPUBLISH:
				break;
			case RtmpManager::SHAKE_RTMP_CREATE_STREAM:
				connection_ptr->Send(RTMP_SERVER_RESULT, sizeof RTMP_SERVER_RESULT);
				break;
			case RtmpManager::SHAKE_RTMP_PUBLISH:
				connection_ptr->Send(RTMP_SERVER_START, sizeof RTMP_SERVER_START);
				break;
			case RtmpManager::SHAKE_SUCCESS:
			{
				connection_ptr->SetNewMessageCallback(OnNewMessage);
				LOG_INFO("connection: %s shake hand success",
						connection_ptr->GetConnectionName().c_str());
				/**
				 * 握手成功时返回
				 */
				return;
			}
			case RtmpManager::SHAKE_FAILED:
			{
				LOG_WARN("connection: %s shake hand failed",
						connection_ptr->GetConnectionName().c_str());
				connection_ptr->CloseConnection();
				/**
				 * 出错时返回
				 */
				return;
			}
			case RtmpManager::SHAKE_DATA_NOT_ENOUGH:
				/**
				 * 数据不足时返回
				 */
				return;
		}
	}
}

int main()
{
	EventLoop loop;
	InetAddress address(4000, true);
	TcpServer server(&loop, "main_server", address);
	server.SetConnectionCallback(OnConnection);
	server.SetNewMessageCallback(OnConnectionShakeHand);
	server.Start();

	loop.Loop();
}
