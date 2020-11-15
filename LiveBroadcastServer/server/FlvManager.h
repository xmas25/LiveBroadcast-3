#ifndef CORE_FLVMANAGER_H
#define CORE_FLVMANAGER_H

#include <vector>

#include "server/codec/FlvCodec.h"
#include "utils/File.h"
#include "utils/Buffer.h"

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

	void Init();

	bool SetFilePath(const std::string& file);

	/**
	 * @brief 解析文件
	 * @param parse_length 最大解析长度
	 * @return 已解析长度 -1出错 否则返回解析长度
	*/
	ssize_t ParseFile(size_t parse_length);

private:

	FlvCodec codec_;

	File file_;

	FlvHeader flv_header_;
	FlvTag video_audio_tags[2];
	FlvTag* current_tag_;
	FlvTag* last_tag_;
	std::vector<FlvTag*> file_tags_;

	ParseStatus parse_status_;
	size_t parsed_length_;

	Buffer buffer_;

	size_t ReadDataFromFile();

	ssize_t ParseHeader();
	ssize_t ParseVideoAudio();
	ssize_t ParseTagHeader();
	ssize_t ParseTagData();

	bool CheckTag();
};

#endif // !CORE_FLVMANAGER_H
