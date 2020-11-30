#ifndef UTILS_CODEC_FLVCODEC_H
#define UTILS_CODEC_FLVCODEC_H

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
#include <memory>
#include "utils/Buffer.h"
/**
 * 负责Flv文件的编码和解码
 *
 * 将Flv文件数据流解码获得 FlvTag Vector
 * 将FlvTag编码成char* 数据流
*/

/**
 * @brief 位于Flv文件头 有且仅有一个
*/
class FlvHeader
{
public:

	/**
	* @brief FlvHeader长度
	*/
	const static int FLV_HEADER_LENGTH = 9;
	static char DEFAULT_HEADER[];

	FlvHeader() :
			flv_("FLV"),
			version_(1),
			type_flag_(5),
			header_length_(0x09000000) // 9的大端序
	{
	}

	void SetInfo(const std::string& flv, uint8_t version, uint8_t type_flag,
			uint32_t header_length)
	{
		flv_ = flv;
		version_ = version;
		type_flag_ = type_flag;
		header_length_ = header_length;
	}

	std::string DebugInfo()
	{
		std::string result = "flv: " + flv_ +
							 ",version: " + std::to_string(version_) +
							 ",type_flag: " + std::to_string(type_flag_) +
							 ",header_length: " + std::to_string(header_length_);

		return result;
	}

	ssize_t EncodeToBuffer(char* data, size_t length);

private:
	std::string flv_; // FLV
	uint8_t version_; // 1
	uint8_t type_flag_; // 5
	uint32_t header_length_; // 9
};

/**
 * @brief FlvTag Flv文件Tag 含Tag头和Tag Data
 * 在Flv文件FlvHeader后 为FlvTag的集合
*/
class FlvTag
{
public:
	/**
	 * @brief FlvTagHeader长度
	*/
	const static int FLV_TAG_HEADER_LENGTH = 15;

	FlvTag();
	~FlvTag();

	/**
	 * @brief 获取本数据包 头数据部分标识的的data_长度
	 * @return
	*/
	uint32_t GetDataSize() const;

	/**
	 * @brief 获取上一个数据包的长度 其中不包含previous_tag_size的四个字节 包含DataSize
	 * @return
	*/
	uint32_t GetPreviousTagSize() const;

	/**
	 * @brief 作用类似GetPreviousTagSize 不过为获取的当前数据包的数据包长度 用于进行校验
	 * @return
	*/
	uint32_t GetCurrentTagSize() const;

	/**
	 * @brief 获取现在已经保存的的data_长度
	 * @return
	*/
	uint32_t GetBodyDataLength() const;

	/**
	 * @brief 获取缺少的data字节数
	 * @return
	*/
	uint32_t GetRemainDataSize() const;

	/**
	 * @brief 追加data数据
	 * @param data data指针
	 * @param length 追加的长度
	*/
	void AppendData(const char* data, size_t length);

	/**
	 * @brief 从数据流中解析出FlvTag的Header部分
	 * @param data 数据流
	 * @param length 数据流长度
	 * @return 成功返回解析长度 解析错误返回-1 数据长度不够返回0
	*/
	ssize_t DecodeTagHander(const char* data, size_t length);

	const Buffer* GetBody() const;
	const char* GetHeader() const;

	void SetTagType(uint8_t tag_type);

	void SetDataSize(const uint8_t* data_size);

	void SetTimeStamp(uint8_t* timestamp);

	void SetSteamId(uint8_t* stream_id);

	void SetPreviousTagSize(uint32_t previous_tag_size);

	uint8_t GetTagType() const;

	Buffer* GetBody();
private:
	//uint32_t previous_tag_size_; // 不含previous_tag_size  sizeof 上一个Tag - 4  大端序保存
	//uint8_t tag_type_; // 音频 8 视频 9 scripts 18
	//uint8_t data_size_[3]; // AudioTag VideoTag 的数据长度 从stream_id后开始算起
	//uint8_t timestamp_[3];
	//uint8_t timestamp_extend_;
	//uint8_t stream_id_[3]; // 0

	const static int PREVIOUS_TAG_SIZE_SUB = 0;
	const static int TAG_TYPE_SUB = 4;
	const static int DATA_SIZE_SUB = 5;
	const static int TIMESTAMP_SUB = 8;
	const static int STREAM_ID_SUB = 10;

	const static int PREVIOUS_TAG_SIZE_LENGTH = 4;
	const static int TAG_TYPE_LENGTH = 1;
	const static int DATA_SIZE_LENGTH = 3;
	const static int TIMESTAMP_LENGTH = 3;
	const static int STREAM_ID_LENGTH = 3;

	/* 由于header会进行多次拷贝 所以为了减少拷贝次数 直接保存序列化后的结果*/
	char header_[15];
	Buffer body_;
};
typedef std::shared_ptr<FlvTag> FlvTagPtr;

/**
 * @brief Flv解码器 在FlvTag和FlvHeader基础上进行包装 提供解析功能
*/
class FlvCodec
{
public:
	FlvCodec() = default;
	~FlvCodec() = default;

	ssize_t DecodeFileHeader(const char* data, size_t length, FlvHeader* tag);
	ssize_t DecodeTagHander(const char* data, size_t length, FlvTag* tag);

private:
};
#endif // !UTILS_CODEC_FLVCODEC_H
