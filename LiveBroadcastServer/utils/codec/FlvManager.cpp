#include <cstdio>
#include <cassert>
#include "utils/codec/FlvManager.h"

FlvManager::FlvManager() :
		FlvManager("")
{
}

FlvManager::FlvManager(const std::string& file) :
		codec_(),
		file_(file),
		flv_header_(),
		video_audio_tags(),
		current_tag_(nullptr),
		last_tag_(nullptr),
		flv_tags_(),
		parse_status_(FlvManager::TAG_HEADER),
		parsed_length_(0),
		buffer_(BUFFER_SIZE)
{

}

FlvManager::~FlvManager()
{
	for (FlvTag* tag : flv_tags_)
	{
		delete tag;
	}
}

bool FlvManager::SetFilePath(const std::string& file)
{
	bool result = file_.Open(file);

	return result;
}

ssize_t FlvManager::ParseFile(size_t parse_length)
{
	/* 可以设置指定的解析长度 但最大为文件长度 -4 Flv最后四个字节用于校验*/
	parse_length = file_.GetFileSize() > parse_length ? parse_length :
				   file_.GetFileSize() - 4;


	ReadDataFromFile();

	/* 首先从数据流解析出 Header Script和音视频Tag*/
	ssize_t parsed = ParseHeader();
	if (parsed < 0)
	{
		printf("parse header error");
		return -1;
	}
	parsed_length_ += parsed;

	parsed = ParseScripTag();
	if (parsed < 0)
	{
		printf("parse script error");
		return -1;
	}
	parsed_length_ += parsed;

	parsed = ParseVideoAudio();
	if (parsed < 0)
	{
		printf("parse video audio error");
		return -1;
	}
	parsed_length_ += parsed;

	current_tag_ = new FlvTag;

	/* 开始解析存储声音和视频的Tag*/
	bool parsing = true;
	while (parsing && parsed_length_ < parse_length)
	{
		if (parse_status_ == FlvManager::TAG_HEADER)
		{
			parsed = ParseTagHeader(current_tag_);
			if (parsed < 0)
			{
				printf("parse tag header error");
				parsing = false;
				break;
			}
			else if (parsed == 0)
			{
				ReadDataFromFile();
				continue;
			}

			/* 成功解析一个TagHeader部分后 进行校验*/
			if (!CheckTag())
			{
				parsing = false;
				break;
			}
		}
		if (parse_status_ == FlvManager::TAG_DATA)
		{
			parsed = ParseTagData(current_tag_);
			if (parsed < 0)
			{
				printf("parse tag data error");
				parsing = false;
			}
			else if (parsed == 0)
			{
				ReadDataFromFile();
				continue;
			}
			last_tag_ = current_tag_;
			flv_tags_.push_back(current_tag_);
			current_tag_ = new FlvTag;

			parsed += FlvTag::FLV_TAG_HEADER_LENGTH;

			parsed_length_ += parsed;
			printf("parse: %zo, sum parsed: %zu\n", parsed, parsed_length_);
		}
	}

	return parsed_length_;
}

FlvTag* FlvManager::GetScriptTag()
{
	return &script_tag_;
}

FlvTag* FlvManager::GetVideoAudioTags()
{
	return &video_audio_tags[0];
}

ssize_t FlvManager::EncodeHeadersToBuffer(Buffer* buffer)
{
	uint32_t data_length = FlvHeader::FLV_HEADER_LENGTH + script_tag_.GetCurrentTagSize() + 4
						   + video_audio_tags[0].GetCurrentTagSize() + 4 + video_audio_tags[1].GetCurrentTagSize() + 4;

	if (buffer->WritableLength() < data_length)
	{
		return 0;
	}

	ssize_t result = flv_header_.EncodeToBuffer(buffer->WriteBegin(), buffer->WritableLength());
	assert(result > 0);
	buffer->AddWriteIndex(result);

	buffer->AppendData(script_tag_.GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
	buffer->AppendData(script_tag_.GetBody());

	video_audio_tags[0].SetPreviousTagSize(script_tag_.GetCurrentTagSize());
	video_audio_tags[1].SetPreviousTagSize(video_audio_tags[0].GetCurrentTagSize());

	for (int i = 0; i < 2; ++i)
	{
		buffer->AppendData(video_audio_tags[i].GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
		buffer->AppendData(video_audio_tags[i].GetBody());
	}

	return data_length;
}

std::vector<FlvTag*>* FlvManager::GetFlvTags()
{
	return &flv_tags_;
}

size_t FlvManager::ReadDataFromFile()
{
	buffer_.AdjustBuffer();

	ssize_t result = file_.Read(buffer_.WriteBegin(), buffer_.WritableLength());
	buffer_.AddWriteIndex(result);

	return result;
}

ssize_t FlvManager::ParseHeader()
{
	ssize_t parse_result = codec_.DecodeFileHeader(buffer_.ReadBegin(), buffer_.ReadableLength(), &flv_header_);
	if (parse_result > 0)
	{
		buffer_.AddReadIndex(parse_result);
	}
	else if (parse_result < 0)
	{
		return -1;
	}

	return parse_result;
}

ssize_t FlvManager::ParseScripTag()
{
	size_t sum_parsed = 0;

	ssize_t parsed = ParseTagHeader(&script_tag_);
	if (parsed <= 0)
	{
		return -1;
	}
	sum_parsed += parsed;

	parsed = ParseTagData(&script_tag_);
	if (parsed <= 0)
	{
		return -1;
	}
	sum_parsed += parsed;

	return sum_parsed;
}

ssize_t FlvManager::ParseVideoAudio()
{
	size_t sum_parsed = 0;

	for (int i = 0; i <= 1; ++i)
	{
		ssize_t parsed = ParseTagHeader(&video_audio_tags[i]);
		if (parsed <= 0)
		{
			return -1;
		}
		sum_parsed += parsed;

		parsed = ParseTagData(&video_audio_tags[i]);
		if (parsed <= 0)
		{
			return -1;
		}
		sum_parsed += parsed;
	}


	return sum_parsed;
}

ssize_t FlvManager::ParseTagHeader(FlvTag* tag)
{
	if (buffer_.ReadableLength() < FlvTag::FLV_TAG_HEADER_LENGTH)
	{
		return 0;
	}

	//last_tag_ = current_tag_;
	//current_tag_ = new FlvTag();

	ssize_t parse_result = codec_.DecodeTagHander(buffer_.ReadBegin(), buffer_.ReadableLength(), tag);
	if (parse_result > 0)
	{
		buffer_.AddReadIndex(parse_result);
	}
	else
	{
		return -1;
	}

	parse_status_ = FlvManager::TAG_DATA;
	return parse_result;
}

ssize_t FlvManager::ParseTagData(FlvTag* tag)
{
	size_t data_size = tag->GetDataSize();
	size_t remain_size = data_size - tag->GetBodyDataLength();
	size_t buffer_size = buffer_.ReadableLength();

	if (remain_size <= buffer_size)
	{
		tag->AppendData(buffer_.ReadBegin(), remain_size);
		buffer_.AddReadIndex(remain_size);
	}
	else
	{
		tag->AppendData(buffer_.ReadBegin(), buffer_size);
		buffer_.AddReadIndex(buffer_size);
		return 0;
	}

	//file_tags_.push_back(tag);
	parse_status_ = FlvManager::TAG_HEADER;
	return data_size;
}

bool FlvManager::CheckTag()
{
	if (!last_tag_)
	{
		return true;
	}

	/* Flv文件中 previous_tag_size为上一个Tag的长度 用于校验 相等则正确 否则校验失败*/

	uint32_t tag_size = last_tag_->GetCurrentTagSize();
	uint32_t previous_tag_size = current_tag_->GetPreviousTagSize();

	if (tag_size != previous_tag_size)
	{
		printf("check : tag_size-%d, previous_tag_size-%d\n", tag_size,
				previous_tag_size);

		return false;
	}

	return true;
}
