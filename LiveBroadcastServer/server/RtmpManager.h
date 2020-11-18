#ifndef SERVER_RTMPMANAGER_H
#define SERVER_RTMPMANAGER_H

#include "server/codec/RtmpCodec.h"
#include "server/FlvManager.h"

constexpr int RTMP_START_PARSE_LENGTH = 1000;
constexpr int RTMP_CHUNK_SIZE = 4096;


class RtmpManager
{
public:

	enum ParseStatus
	{
		PARSE_RTMP_FIRST_HEADER,
		PARSE_RTMP_HEADER,
		PARSE_RTMP_BODY
	};

	RtmpManager();
	~RtmpManager();

	ssize_t ParseData(Buffer* buffer);

	FlvManager* GetFlvManager();
private:

	ParseStatus parsed_status_;

	size_t parsed_length_;

	RtmpPack rtmp_pack_;
	RtmpCodec rtmp_codec_;

	FlvManager flv_manager_;

	FlvTag* current_tag_;

	/* 由于chunk的分块存在 导致 当body大于4096字节时, 每读取4096个字节 需要重新解析一次header故在此记录*/
	uint32_t read_chunk_size_;
	/* 用于标识当前chunk是否解析完毕 未解析完不能创建新的tag*/
	bool chunk_over_;

	ssize_t ParseFirstHeader(Buffer* buffer);
	ssize_t ParseScriptPack(Buffer* buffer);
	ssize_t ParseVideoAudio(Buffer* buffer);

	ssize_t ParseHeader(Buffer* buffer);

	ssize_t ParseBody(Buffer* buffer);

	void PushBackCurrentFlvTag();
};

#endif