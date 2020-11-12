#ifndef UTILS_CODEC_H
#define UTILS_CODEC_H

#include <stdint.h>

class FileHeader
{
	char flv[3]; // FLV
	uint8_t version; // 1
	uint8_t type_flags; // 5
	uint32_t header_length; // 9
};

class FileTag
{
	uint32_t previous_tag_size; // 不含previous_tag_size  sizeof 上一个Tag - 4
	uint8_t tag_type; // 音频 8 视频 9 scripts 18
	uint8_t data_size[3]; // AudioTag VideoTag 的数据长度 从stream_id后开始算起
	uint8_t timestamp[3];
	uint8_t timestamp_extend;
	uint8_t stream_id[3]; // 0

	char* data;
};

FileHeader file_header;
FileTag info_tag;
FileTag sps_pps_tag;

class Codec
{


};

#endif // !UTILS_CODEC_H
