#include <cstdio>
#include "server/FlvManager.h"

FlvManager::FlvManager() :
	FlvManager("")
{
}

FlvManager::FlvManager(const std::string& file) :
	codec_(),
	file_(file),
	flv_header_(),
	flv_sps_pps_(),
	current_tag_(nullptr),
	file_tags_(),
	buffer_(BUFFER_SIZE)
{
	Init();
}

FlvManager::~FlvManager()
{
}

void FlvManager::Init()
{
	parse_status_ = FlvManager::TAG_HEADER;

	parsed_length_ = 0;
}

bool FlvManager::SetFilePath(const std::string& file)
{
	bool result = file_.Open(file);

	if (result)
	{
		Init();
	}
	
	return result;
}

ssize_t FlvManager::ParseFile(size_t parse_length)
{
	parse_length = file_.GetFileSize() > parse_length ? parse_length :
		file_.GetFileSize() - 4;


	ReadDataFromFile();
	ssize_t parsed = ParseHeader();
	if (parsed < 0)
	{
		printf("parse header error");
		return -1;
	}
	parsed_length_ += parsed;

	parsed = ParseSpsPps();
	if (parsed < 0)
	{
		printf("parse sps pps error");
		return -1;
	}
	parsed_length_ += parsed;

	bool parsing = true;
	while (parsing && parsed_length_ < parse_length)
	{
		if (parse_status_ == FlvManager::TAG_HEADER)
		{
			parsed = ParseTagHeader();
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
			if (!CheckTag())
			{
				parsing = false;
				break;
			}
		}
		if (parse_status_ == FlvManager::TAG_DATA)
		{
			parsed = ParseTagData();
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
			parsed += FLV_TAG_HEADER_LENGTH;

			parsed_length_ += parsed;
			printf("parse: %zo, sum parsed: %zu\n", parsed, parsed_length_);
		}
	}

	return parsed_length_;
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

ssize_t FlvManager::ParseSpsPps()
{
	ssize_t parse_result = codec_.DecodeTagHander(buffer_.ReadBegin(), buffer_.ReadableLength(), &flv_sps_pps_);
	if (parse_result > 0)
	{
		buffer_.AddReadIndex(parse_result);
	}
	else if (parse_result < 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}

	uint32_t data_size = flv_sps_pps_.GetDataSize();
	if (buffer_.ReadableLength() >= data_size)
	{
		flv_sps_pps_.SetData(buffer_.ReadBegin(), data_size);
		buffer_.AddReadIndex(data_size);
		parse_result += data_size;
	}
	else
	{
		return -1;
	}

	return parse_result;
}

ssize_t FlvManager::ParseTagHeader()
{
	if (buffer_.ReadableLength() < FLV_TAG_HEADER_LENGTH)
	{
		return 0;
	}

	last_tag_ = current_tag_;
	current_tag_ = new FlvTag();

	ssize_t parse_result = codec_.DecodeTagHander(buffer_.ReadBegin(), buffer_.ReadableLength(), current_tag_);
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

ssize_t FlvManager::ParseTagData()
{
	size_t data_size = current_tag_->GetDataSize();
	size_t remain_size = data_size - current_tag_->GetCurrentDataSize();
	size_t buffer_size = buffer_.ReadableLength();

	if (remain_size <= buffer_size)
	{
		current_tag_->AppendData(buffer_.ReadBegin(), remain_size);
		buffer_.AddReadIndex(remain_size);
	}
	else
	{
		current_tag_->AppendData(buffer_.ReadBegin(), buffer_size);
		buffer_.Reset();
		return 0;
	}

	file_tags_.push_back(current_tag_);
	parse_status_ = FlvManager::TAG_HEADER;

	return data_size;
}

bool FlvManager::CheckTag()
{
	if (!last_tag_)
	{
		return true;
	}

	uint32_t tag_size = last_tag_->GetTagSize();
	uint32_t previous_tag_size = current_tag_->GetPreviousTagSize();

	if (tag_size != previous_tag_size)
	{
		printf("check : tag_size-%d, previous_tag_size-%d\n", tag_size,
			previous_tag_size);

		return false;
	}
	
	return true;
}
