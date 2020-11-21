#include "RtmpManager.h"

RtmpManager::RtmpManager():
	parsed_status_(RtmpManager::PARSE_RTMP_FIRST_HEADER),
	parsed_length_(0),
	rtmp_pack_(),
	rtmp_codec_(),
	flv_manager_(),
	read_chunk_size_(0),
	chunk_over_(true)
{
}

RtmpManager::~RtmpManager()
{
}

ssize_t RtmpManager::ParseData(Buffer* buffer)
{
	if (!buffer)
	{
		return -1;
	}

	ssize_t current_loop_parsed = 0;
	ssize_t parsed = 0;

	while (true)
	{
		if (parsed_status_ == RtmpManager::PARSE_RTMP_BODY)
		{
			parsed = ParseBody(buffer);

			if (parsed > 0)
			{
				PushBackCurrentFlvTag();
			}
			else if (parsed < 0)
			{
				printf("ParseBody error\n");
			}
		}
		else
		{
			if (parsed_status_ == RtmpManager::PARSE_RTMP_HEADER)
			{
				parsed = ParseHeader(buffer);
				if (parsed < 0)
				{
					printf("ParseHeader error\n");
				}

			}
			else if (parsed_status_ == RtmpManager::PARSE_RTMP_FIRST_HEADER)
			{
				parsed = ParseFirstHeader(buffer);
				if (parsed < 0)
				{
					printf("ParseFirstHeader error\n");
				}

			}
		}

		if (parsed < 0)
		{
			parsed_length_ += current_loop_parsed;
			return -1;
		}
		else if (parsed == 0)
		{
			break;
		}
		current_loop_parsed += parsed;
		
	}

	parsed_length_ += current_loop_parsed;
	return current_loop_parsed;
}

FlvManager* RtmpManager::GetFlvManager()
{
	return &flv_manager_;
}

ssize_t RtmpManager::ParseFirstHeader(Buffer* buffer)
{
	if (buffer->ReadableLength() < RTMP_START_PARSE_LENGTH)
	{
		return 0;
	}

	ssize_t parsed_script = ParseScriptPack(buffer);
	if (parsed_script < 0)
	{
		return -1;
	}
	
	ssize_t parsed_video_audio = ParseVideoAudio(buffer);
	if (parsed_video_audio < 0)
	{
		return -1;
	}

	parsed_status_ = PARSE_RTMP_HEADER;
	return parsed_script + parsed_video_audio;
}

ssize_t RtmpManager::ParseScriptPack(Buffer* buffer)
{
	ssize_t result = 0;
	for (;;)
	{
		ssize_t parsed = rtmp_codec_.DecodeHeader(buffer->ReadBegin(), buffer->ReadableLength(), &rtmp_pack_);
		buffer->AddReadIndex(parsed);
		result += parsed;

		if (parsed <= 0)
		{
			return -1;
		}

		if (rtmp_pack_.GetRtmpPackType() != RtmpPack::RTMP_SCRIPT)
		{
			buffer->AddReadIndex(rtmp_pack_.GetDataSize());
			result += rtmp_pack_.GetDataSize();
		}
		else
		{
			break;
		}
	}

	FlvTag* script_tag = flv_manager_.GetScriptTag();

	/*
	将rtmp_pack的header部分编码到FlvTag中 然后从buffer拷贝data到FlvTag中减少拷贝次数
	*/
	rtmp_pack_.EncodeHeaderToFlvTag(script_tag);
	script_tag->AppendData(buffer->ReadBegin(), script_tag->GetDataSize());
	buffer->AddReadIndex(script_tag->GetDataSize());
	result += script_tag->GetDataSize();

	

	return result;
}

ssize_t RtmpManager::ParseVideoAudio(Buffer* buffer)
{
	/**
	* 就Obs推流的抓包结果来看 音视频的一个Tag是连着的 这里简化处理 如果不连续则返回错误
	*/
	ssize_t result = 0;
	ssize_t parsed = 0;

	/**
	 * tag为一个两个元素的数组指针 两个元素分别为第一个视频和音频Tag
	*/
	FlvTag* tag = flv_manager_.GetVideoAudioTags();

	for (int i = 0; i < 2; ++i)
	{
		parsed = rtmp_codec_.DecodeHeader(buffer->ReadBegin(), buffer->ReadableLength(), &rtmp_pack_);
		buffer->AddReadIndex(parsed);
		result += parsed;
		if (parsed <= 0)
		{
			return -1;
		}

		if (rtmp_pack_.GetRtmpPackType() == RtmpPack::RTMP_AUDIO || 
			rtmp_pack_.GetRtmpPackType() == RtmpPack::RTMP_VIDEO)
		{
			rtmp_pack_.EncodeHeaderToFlvTag(&tag[i]);
			tag[i].AppendData(buffer->ReadBegin(), tag[i].GetDataSize());
			buffer->AddReadIndex(rtmp_pack_.GetDataSize());
			result += rtmp_pack_.GetDataSize();
		}
		else
		{
			// 简化处理 如果不连续则返回错误
			return -1;
		}
	}

	return result;
}

ssize_t RtmpManager::ParseHeader(Buffer* buffer)
{
	ssize_t parsed = rtmp_codec_.DecodeHeader(buffer->ReadBegin(), buffer->ReadableLength(), &rtmp_pack_);
	if (parsed < 0)
	{
		printf("ParseHeader-DecodeHeader error\n");
		return -1;
	}
	else if (parsed == 0)
	{
		return 0;
	}
	buffer->AddReadIndex(parsed);

	/**
	 * 当前chunk未结束的时候 会继续发送数据 append到当前的tag即可 不应该new一个新的
	*/
	if (chunk_over_)
	{
		current_tag_ = new FlvTag;
		rtmp_pack_.EncodeHeaderToFlvTag(current_tag_);
	}
	

	parsed_status_ = RtmpManager::PARSE_RTMP_BODY;

	return parsed;
}

ssize_t RtmpManager::ParseBody(Buffer* buffer)
{
	
	// TODO 过滤掉非音频数据包的内容 由于过滤仍需丢弃body数据 这里暂时简化

	uint8_t csid = rtmp_pack_.GetCsid();
	if (csid != MOVIE_CSID)
	{
		if (csid == 3 && rtmp_pack_.GetDataSize() == 31)
		{
			printf("parse maybe success!\n");
			return -1;
		}
		else
		{
			printf("rtmp_pack_.GetCsid: %u != MOVIE_CSID\n", rtmp_pack_.GetCsid());
			return -1;
		}
	}

	size_t readable = buffer->ReadableLength();
	size_t remain = current_tag_->GetRemainDataSize();

	// 只有在读满一个chunk分块4096字节后 返回解析一个新的header的时候
	// 当remain小于等于RTMP_CHUNK_SIZE的时候说明 此chunk分块结束了
	if (remain <= RTMP_CHUNK_SIZE && read_chunk_size_ == 0)
	{
		chunk_over_ = true;
	}
	else
	{
		chunk_over_ = false;
	}

	if (chunk_over_)
	{
		// 当前chunk没有分块 或者最后一个chunk分块被接收

		if (readable < remain)
		{
			current_tag_->AppendData(buffer->ReadBegin(), readable);
			buffer->Reset();
			return 0;
		}
		else
		{
			current_tag_->AppendData(buffer->ReadBegin(), remain);
			buffer->AddReadIndex(remain);

			parsed_status_ = RtmpManager::PARSE_RTMP_HEADER;
			return current_tag_->GetDataSize();
		}
	}
	else
	{
		// 当前chunk分块没有全部接受
		size_t current_chunk_remain = RTMP_CHUNK_SIZE - read_chunk_size_;
		if (readable < current_chunk_remain)
		{
			current_tag_->AppendData(buffer->ReadBegin(), readable);
			buffer->AddReadIndex(readable);
			read_chunk_size_ += readable;
		}
		else
		{
			current_tag_->AppendData(buffer->ReadBegin(), current_chunk_remain);
			buffer->AddReadIndex(current_chunk_remain);

			/* chunk 结束 清除当前chunk已读字节数*/
			read_chunk_size_ = 0;

			parsed_status_ = RtmpManager::PARSE_RTMP_HEADER;
		}
		// 当前chunk分块结束 但是chunk未结束 需要继续解析Header 判断接下来是多少字节要Append入Data
		return 0;
	}
}

void RtmpManager::PushBackCurrentFlvTag()
{
	flv_manager_.PushBackFlvTagAndSetPreviousSize(current_tag_);
}
