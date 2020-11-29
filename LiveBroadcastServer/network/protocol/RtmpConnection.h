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

	ShakeHandResult ShakeHand(Buffer* buffer);

	/**
	 * 将当前Tag数据保存到文件中
	 * @param file_write 与保存到的文件
	 * @return 写入的字节数
	 */
	ssize_t WriteToFile(File* file_write);

	ssize_t ParseData(Buffer* buffer);
private:

	TcpConnectionPtr connection_ptr_;

	RtmpManager rtmp_manager_;

	size_t last_write_size_;

	void DebugParseSize(size_t division);
};


#endif //LIVEBROADCASTSERVER_RTMPCONNECTION_H
