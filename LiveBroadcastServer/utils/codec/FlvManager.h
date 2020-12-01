#ifndef CORE_FLVMANAGER_H
#define CORE_FLVMANAGER_H

#include <vector>
#include "utils/codec/FlvCodec.h"
#include "utils/File.h"
#include "utils/Buffer.h"

/**
 * 从Flv文件中解析出数据
 * 用于管理FlvHeader和FlvTag数据
*/

/**
 * @brief 文件读取缓冲区长度
*/
constexpr size_t BUFFER_SIZE = 30000;
class FlvManager
{
public:
	enum ParseStatus
	{
		TAG_HEADER,
		TAG_DATA
	};

	FlvManager();
	FlvManager(const std::string& file);
	~FlvManager();

	bool SetFilePath(const std::string& file);

	/**
	 * @brief 解析文件
	 * @param parse_length 最大解析长度
	 * @return 已解析长度 -1出错 否则返回解析长度
	*/
	ssize_t ParseFile(size_t parse_length);

	/**
	 * @brief 获取ScriptTag的指针
	 * @return
	*/
	FlvTag* GetScriptTag();

	/**
	 * @brief video_audio_tags 指针
	 * @return
	*/
	FlvTag* GetVideoAudioTags();

	/**
	 * TODO 进行缓存
	 * @brief 将FlvHeader script_tag_ 和第一个音频以及一个视频信息编码到缓冲区中
	 * 即使编码成功也不清除上述四者内容 方便继续使用
	 * @param buffer
	 * @return 成功返回序列化的字节数 失败返回-1 缓冲区长度不足返回0
	*/
	ssize_t EncodeHeadersToBuffer(Buffer* buffer);

	std::vector<FlvTag*>* GetFlvTags();
private:

	FlvCodec codec_;

	File file_;

	/* 每个Flv文件有且仅有一个*/
	FlvHeader flv_header_;

	FlvTag script_tag_;

	/* 第一个音频和视频tag存储着编码信息 经常使用需要独立存储*/
	FlvTag video_audio_tags[2];

	/* 当前正在处理的tag*/
	FlvTag* current_tag_;

	/* 上一个处理过的tag*/
	FlvTag* last_tag_;

	/* tag集合*/
	std::vector<FlvTag*> flv_tags_;


	ParseStatus parse_status_;

	/* 已经解析的长度*/
	size_t parsed_length_;

	Buffer buffer_;

	size_t ReadDataFromFile();

	ssize_t ParseHeader();
	ssize_t ParseScripTag();
	ssize_t ParseVideoAudio();
	ssize_t ParseTagHeader(FlvTag* tag);
	ssize_t ParseTagData(FlvTag* tag);

	bool CheckTag();
};

#endif // !CORE_FLVMANAGER_H
