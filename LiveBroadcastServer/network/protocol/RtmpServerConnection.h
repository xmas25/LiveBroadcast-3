//
// Created by rjd67 on 2020/11/29.
//

#ifndef LIVEBROADCASTSERVER_RTMPSERVERCONNECTION_H
#define LIVEBROADCASTSERVER_RTMPSERVERCONNECTION_H

#include <map>

#include "network/TcpConnection.h"
#include "utils/codec/RtmpManager.h"
#include "network/protocol/RtmpClientConnection.h"

/**
 * 管理Tcp连接 和 RtmpManager
 * 管理FlvManager的Tag缓冲区 实时替换
 */
class RtmpServerConnection;
typedef std::shared_ptr<RtmpClientConnection> RtmpClientConnectionPtr;
typedef std::map<std::string, RtmpClientConnectionPtr> RtmpClientConnectionMap;
typedef std::function<void(RtmpServerConnection*)> ShakeHandSuccessCallback;
typedef std::function<bool(const std::string&, const std::string&)> AuthenticationCallback;

class RtmpServerConnection
{
public:
	enum ShakeHandResult
	{
		SHAKE_AUTHENTICATE,
		SHAKE_SUCCESS,
		SHAKE_FAILED,
		SHAKE_DATA_NOT_ENOUGH
	};

	explicit RtmpServerConnection(const TcpConnectionPtr& connection_ptr);

	ssize_t ParseData(Buffer* buffer);

	/**
	 * 获取Flv直播流的源数据包
	 * @return
	 */
	const Buffer* GetHeaderDataBuffer();

	/**
	 * 用于握手的回调函数  握手成功后自动切换为OnBodyData 处理真正的数据
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

	/**
	 * 增加一个观看者连接到 推流者连接
	 *
	 * 发送头部数据 保存观看者连接
	 * @param client_connection_ptr
	 */
	void AddClientConnection(const RtmpClientConnectionPtr& client_connection_ptr);

	void SetShakeHandSuccessCallback(const ShakeHandSuccessCallback& callback);

	std::string GetRtmpUrl() const;

	/**
	 * xxx/aaa -> aaa
	 * @return
	 */
	std::string GetRtmpPath() const;

	std::string GetConnectionName() const;

	void SetAuthenticationCallback(const AuthenticationCallback& callback);
private:

	TcpConnectionPtr connection_ptr_;
	RtmpClientConnectionMap client_connection_map_;

	RtmpManager rtmp_manager_;
	FlvManager* flv_manager_;

	size_t last_write_size_;

	FlvTagPtr last_flv_tag_ptr_;

	/**
	 * Flv流源数据缓冲区
	 */
	Buffer header_buffer_;

	ShakeHandSuccessCallback shake_hand_success_callback_;

	AuthenticationCallback authentication_callback_;

	void DebugParseSize(size_t division);

	/**
	 * 解析握手数据
	 * @param buffer 主播端发送的握手数据
	 * @return 当前的握手装备
	 */
	ShakeHandResult ShakeHand(Buffer* buffer);

	/**
	 * 向一个新加入的连接 发送头部数据
	 * @param client_connection_ptr 新加入的连接
	 */
	void SendHeaderToClientConnection(const RtmpClientConnectionPtr& client_connection_ptr);

	/**
	 * 新FlvTag的回调函数
	 * @param new_tag
	 */
	void OnNewFlvTag(const FlvTagPtr& new_tag);

	uint32_t GetLastHeaderTagCurrentSize() const;

	void OnConnectionClose(const TcpConnectionPtr& connection_ptr);

	bool Authenticate();
};


#endif //LIVEBROADCASTSERVER_RTMPSERVERCONNECTION_H
