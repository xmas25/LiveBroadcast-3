//
// Created by rjd67 on 2020/11/29.
//

#ifndef LIVEBROADCASTSERVER_RTMPCONNECTION_H
#define LIVEBROADCASTSERVER_RTMPCONNECTION_H

#include "network/TcpConnection.h"
#include "utils/codec/RtmpManager.h"

/**
 * 管理Tcp连接 和 RtmpManager
 * 管理FlvManager的Tag缓冲区 实时替换
 */
class RtmpConnection
{
public:
	enum ShakeHandResult
	{
		SHAKE_SUCCESS,
		SHAKE_FAILED,
		SHAKE_DATA_NOT_ENOUGH
	};

	RtmpConnection(const TcpConnectionPtr& connection_ptr);

	/**
	 * 将当前Tag数据保存到文件中
	 * @param file_write 与保存到的文件
	 * @return 写入的字节数
	 */
	ssize_t WriteToFile(File* file_write);

	ssize_t ParseData(Buffer* buffer);

	const Buffer* GetHeaderDataBuffer();

	/**
	 * 用于握手的回调函数
	 * @param connection_ptr
	 * @param buffer
	 * @param timestamp
	 */
	void OnConnectionShakeHand(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp);

	/**
	 * 握手结束后 处理数据的回调函数
	 * @param connection_ptr
	 * @param buffer
	 * @param timestamp
	 */
	void OnBodyData(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp);

private:

	TcpConnectionPtr connection_ptr_;

	RtmpManager rtmp_manager_;
	FlvManager* flv_manager_;

	size_t last_write_size_;

	Buffer header_buffer_;
	std::vector<FlvTag*>* flv_tag_vector_;
	void DebugParseSize(size_t division);

	ShakeHandResult ShakeHand(Buffer* buffer);
};


#endif //LIVEBROADCASTSERVER_RTMPCONNECTION_H
