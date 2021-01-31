#ifndef SERVER_RTMPMANAGER_H
#define SERVER_RTMPMANAGER_H

#include <functional>
#include "utils/codec/RtmpCodec.h"
#include "utils/codec/FlvManager.h"

constexpr int RTMP_START_PARSE_LENGTH = 1000;
constexpr int RTMP_CHUNK_SIZE = 4096;

typedef std::function<void(const FlvTagPtr&)> NewFlvTagCallback;
class RtmpManager
{
public:

	enum ParseStatus
	{
		PARSE_FIRST_HEADER,
		PARSE_RTMP_HEADER,
		PARSE_RTMP_BODY
	};

	/**
	 *
	 */
	enum ShakeHandPackType
	{
		SHAKE_RTMP_C01,
		SHAKE_RTMP_C2,
		SHAKE_RTMP_SET_CHUNK_SIZE,
		SHAKE_RTMP_CONNECT,
		SHAKE_RTMP_RELEASE_STREAM,
		SHAKE_RTMP_FCPUBLISH,
		SHAKE_RTMP_CREATE_STREAM,
		SHAKE_RTMP_PUBLISH,
		SHAKE_SUCCESS,
		SHAKE_FAILED,
		SHAKE_DATA_NOT_ENOUGH
	};


	RtmpManager();
	~RtmpManager();

	ssize_t ParseData(Buffer* buffer);

	FlvManager* GetFlvManager();

	/**
	 * 解析握手数据
	 * @param buffer
	 * @return 此轮解析时的状态
	 */
	ShakeHandPackType ParseShakeHand(Buffer* buffer);

	size_t GetParsedLength() const;

	void SetNewFlvTagCallback(const NewFlvTagCallback& callback);

	std::string GetUrlFromConnectPack() const;

	std::string GetPasswordFromReleasePack();

private:

	ParseStatus parsed_status_;
	ShakeHandPackType shake_hand_status_;

	size_t parsed_length_;

	RtmpCodec rtmp_codec_;

	FlvManager flv_manager_;

	RtmpPack current_rtmp_pack_;

	/* 由于chunk的分块存在 导致 当body大于4096字节时, 每读取4096个字节 需要重新解析一次header故在此记录*/
	uint32_t read_chunk_size_;
	/* 用于标识当前chunk是否解析完毕 未解析完不能创建新的tag*/
	bool chunk_over_;

	NewFlvTagCallback new_flv_tag_callback_;

	FlvTagPtr last_flv_ptr_;

	RtmpPack connect_pack_;

	RtmpPack release_pack_;

	ssize_t ParseFirstHeader(Buffer* buffer);
	ssize_t ParseScriptPack(Buffer* buffer, RtmpPack* script_pack);
	ssize_t ParseVideoAudio(Buffer* buffer, RtmpPack video_audio_pack[2]);

	ssize_t ParseBody(Buffer* buffer);

	/**
	 * 从Buffer中解析 Header, 不移动Buffer读指针
	 * @param buffer
	 * @param rtmp_pack
	 * @return 成功返回解析的长度 数据不足返回-1
	 */
	static ssize_t ParseHeader(const Buffer* buffer, RtmpPack* rtmp_pack);

	static ssize_t ParseBody(Buffer* buffer, RtmpPack* rtmp_pack, bool chunk_over,
			uint32_t* read_chunk_size, ParseStatus* parsed_status);

	/**
	 * 解析头部后 解析body  如果解析头部或body时长度不足则返回0 不移动读指针
	 * 如果完成解析则移动读指针
	 * @param buffer
	 * @param rtmp_pack
	 * @return 解析成功返回解析长度 长度不足返回0 失败返回-1
	 */
	static ssize_t ParseHeaderAndBody(Buffer* buffer, RtmpPack* rtmp_pack);

	void ProcessNewFlvTag(const FlvTagPtr& tag);
};

#endif