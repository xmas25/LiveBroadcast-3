#include <cassert>
#include <iostream>
#include "utils/codec/RtmpManager.h"
#include "utils/Format.h"
#include "utils/Logger.h"

RtmpManager::RtmpManager():
		parsed_status_(RtmpManager::PARSE_FIRST_HEADER),
		shake_hand_status_(RtmpManager::SHAKE_RTMP_C01),
		parsed_length_(0),
		rtmp_codec_(),
		flv_manager_(),
		read_chunk_size_(0),
		chunk_over_(true),
		last_flv_ptr_()
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

	if (buffer->ReadableLength() == 0)
	{
		return 0;
	}

	ssize_t current_loop_parsed = 0;
	ssize_t parsed = 0;

	while (true)
	{
		if (parsed_status_ == RtmpManager::PARSE_RTMP_HEADER)
		{
			parsed = ParseHeader(buffer, &current_rtmp_pack_);
			if (parsed > 0)
			{
				buffer->AddReadIndex(parsed);
				parsed_status_ = RtmpManager::PARSE_RTMP_BODY;
			}
			else if (parsed < 0)
			{
				LOG_ERROR("ParseHeader error");
			}
		}

		if (parsed_status_ == RtmpManager::PARSE_RTMP_BODY)
		{
			parsed = ParseBody(buffer);
			if (parsed > 0)
			{
				// DEBUG信息 LOG_INFO("%s", current_rtmp_pack_.GetHeaderDebugMessage().c_str());

				FlvTagPtr tag_ptr = std::make_shared<FlvTag>();
				rtmp_codec_.EncodeHeaderAndSwapBuffer(&current_rtmp_pack_, tag_ptr.get());
				ProcessNewFlvTag(tag_ptr);
			}
			else if (parsed < 0)
			{
				LOG_ERROR("ParseBody error");
			}
		}
		else if (parsed_status_ == PARSE_FIRST_HEADER)
		{
			parsed = ParseFirstHeader(buffer);
			if (parsed < 0)
			{
				LOG_ERROR("ParseFirstHeader error");
			}
		}

		if (parsed > 0)
		{
			current_loop_parsed += parsed;
		}
		else if (parsed == 0)
		{
			break;
		}
		else
		{
			return -1;
		}
	}

	parsed_length_ += current_loop_parsed;
	return current_loop_parsed;
}

FlvManager* RtmpManager::GetFlvManager()
{
	return &flv_manager_;
}

void RtmpManager::SetNewFlvTagCallback(const NewFlvTagCallback& callback)
{
	new_flv_tag_callback_ = callback;
}

std::string RtmpManager::GetUrlFromConnectPack() const
{
	RtmpPack connect_pack = connect_pack_;
	std::string body_str = connect_pack.GetBuffer()->ReadAllAsString();
	auto begin_idx = body_str.find("rtmp");
	size_t len = strlen(&body_str[begin_idx]);
	return body_str.substr(begin_idx, len);
}

std::string RtmpManager::GetPasswordFromReleasePack()
{
	const char* release_buffer_begin = release_pack_.GetBuffer()->ReadBegin();
	int len = release_buffer_begin[28];
	return std::string(&release_buffer_begin[29], len);
}

ssize_t RtmpManager::ParseFirstHeader(Buffer* buffer)
{
	/**
	 * 一次将 脚本包 第一个音频 第一个视频解析出来
	 */
	if (buffer->ReadableLength() < RTMP_START_PARSE_LENGTH)
	{
		return 0;
	}

	/**
	 * 解析脚本包并保存到flv_manager_
	 */
	RtmpPack script_pack;
	ssize_t parsed_script = ParseScriptPack(buffer, &script_pack);
	if (parsed_script < 0)
	{
		return -1;
	}
	FlvTag* script_tag = flv_manager_.GetScriptTag();
	rtmp_codec_.EncodeHeaderAndSwapBuffer(&script_pack, script_tag);

	/**
	 * 解析第一个视频和音频包保存到flv_manager_
	 */
	RtmpPack video_audio_pack[2];
	ssize_t parsed_video_audio = ParseVideoAudio(buffer, video_audio_pack);
	if (parsed_video_audio < 0)
	{
		return -1;
	}
	FlvTag* video_audio_tag = flv_manager_.GetVideoAudioTags();
	for (int i = 0; i < 2; ++i)
	{
		rtmp_codec_.EncodeHeaderAndSwapBuffer(&video_audio_pack[i], &video_audio_tag[i]);
	}

	parsed_status_ = PARSE_RTMP_HEADER;
	return parsed_script + parsed_video_audio;
}

ssize_t RtmpManager::ParseScriptPack(Buffer* buffer, RtmpPack* script_pack)
{
	ssize_t result = 0;

	ssize_t parsed = RtmpCodec::DecodeHeader(buffer->ReadBegin(), buffer->ReadableLength(), script_pack);
	buffer->AddReadIndex(parsed);
	result += parsed;

	if (parsed <= 0)
	{
		return -1;
	}

	if (script_pack->GetRtmpPackType() != RtmpPack::RTMP_SCRIPT)
	{
		return -1;
	}
	else
	{
		script_pack->AppendData(buffer->ReadBegin(), script_pack->GetBodyDataSize());
		buffer->AddReadIndex(script_pack->GetBodyDataSize());
		result += script_pack->GetBodyDataSize();
	}

	return result;
}

ssize_t RtmpManager::ParseVideoAudio(Buffer* buffer, RtmpPack video_audio_pack[2])
{
	/**
	* 就Obs推流的抓包结果来看 音视频的一个Tag是连着的 这里简化处理 如果不连续则返回错误
	*/
	ssize_t result = 0;
	ssize_t parsed;

	for (int i = 0; i < 2; ++i)
	{
		parsed = RtmpCodec::DecodeHeader(buffer->ReadBegin(), buffer->ReadableLength(), &video_audio_pack[i]);
		buffer->AddReadIndex(parsed);
		result += parsed;
		if (parsed <= 0)
		{
			return -1;
		}

		if (video_audio_pack[i].GetRtmpPackType() == RtmpPack::RTMP_AUDIO ||
				video_audio_pack[i].GetRtmpPackType() == RtmpPack::RTMP_VIDEO)
		{
			size_t data_size = video_audio_pack[i].GetBodyDataSize();
			video_audio_pack[i].AppendData(buffer->ReadBegin(), data_size);
			buffer->AddReadIndex(data_size);
			result += data_size;
		}
		else
		{
			// 简化处理 如果不连续则返回错误
			return -1;
		}
	}

	return result;
}

ssize_t RtmpManager::ParseHeader(const Buffer* buffer, RtmpPack* rtmp_pack)
{
	ssize_t parsed = RtmpCodec::DecodeHeader(buffer->ReadBegin(), buffer->ReadableLength(), rtmp_pack);
	if (parsed < 0)
	{
		printf("ParseHeader-DecodeHeader error\n");
		return -1;
	}
	else if (parsed == 0)
	{
		return 0;
	}
	return parsed;
}

ssize_t RtmpManager::ParseBody(Buffer* buffer)
{
	// 只有在读满一个chunk分块4096字节后 返回解析一个新的header的时候
	// 当remain小于等于RTMP_CHUNK_SIZE的时候说明 此chunk分块结束了
	if (current_rtmp_pack_.GetBodyRemainSize() <=
		RTMP_CHUNK_SIZE && (read_chunk_size_ == 0))
	{
		chunk_over_ = true;
	}
	else
	{
		chunk_over_ = false;
	}

	return ParseBody(buffer, &current_rtmp_pack_, chunk_over_, &read_chunk_size_, &parsed_status_);
}
ssize_t RtmpManager::ParseBody(Buffer* buffer, RtmpPack* rtmp_pack, bool chunk_over,
		uint32_t* read_chunk_size, ParseStatus* parsed_status)
{
	size_t readable = buffer->ReadableLength();
	size_t remain = rtmp_pack->GetBodyRemainSize();

	if (chunk_over)
	{
		// 当前chunk没有分块 或者最后一个chunk分块被接收

		if (readable < remain)
		{
			rtmp_pack->AppendData(buffer->ReadBegin(), readable);
			buffer->AddReadIndex(readable);
			return 0;
		}
		else
		{
			rtmp_pack->AppendData(buffer->ReadBegin(), remain);
			buffer->AddReadIndex(remain);

			if (parsed_status)
			{
				*parsed_status = RtmpManager::PARSE_RTMP_HEADER;
			}
			return rtmp_pack->GetBodyDataSize();
		}
	}
	else
	{
		// 当前chunk分块没有全部接受
		size_t current_chunk_remain = RTMP_CHUNK_SIZE - *read_chunk_size;
		if (readable < current_chunk_remain)
		{
			rtmp_pack->AppendData(buffer->ReadBegin(), readable);
			buffer->AddReadIndex(readable);
			*read_chunk_size += readable;
		}
		else
		{
			rtmp_pack->AppendData(buffer->ReadBegin(), current_chunk_remain);
			buffer->AddReadIndex(current_chunk_remain);

			/* chunk 结束 清除当前chunk已读字节数*/
			*read_chunk_size = 0;

			*parsed_status = RtmpManager::PARSE_RTMP_HEADER;
		}
		// 当前chunk分块结束 但是chunk未结束 需要继续解析Header 判断接下来是多少字节要Append入Data
		return 0;
	}
}

void RtmpManager::ProcessNewFlvTag(const FlvTagPtr& tag_ptr)
{
	if (tag_ptr->GetTagType() == RtmpPack::RTMP_VIDEO || tag_ptr->GetTagType() == RtmpPack::RTMP_AUDIO)
	{
		if (last_flv_ptr_)
		{
			tag_ptr->SetPreviousTagSize(last_flv_ptr_->GetCurrentTagSize());
		}
		else
		{
			tag_ptr->SetPreviousTagSize(0);
		}
		last_flv_ptr_ = tag_ptr;

		if (new_flv_tag_callback_)
		{
			new_flv_tag_callback_(tag_ptr);
		}
	}
}

RtmpManager::ShakeHandPackType RtmpManager::ParseShakeHand(Buffer* buffer)
{
	ssize_t parse = 0;
	switch (shake_hand_status_)
	{
		case SHAKE_RTMP_C01:
		{
			if (buffer->ReadableLength() < 1537)
			{
				return SHAKE_DATA_NOT_ENOUGH;
			}
			else
			{
				shake_hand_status_ = SHAKE_RTMP_C2;
				buffer->AddReadIndex(1537);
				return SHAKE_RTMP_C01;
			}

		}
		case SHAKE_RTMP_C2:
		{
			if (buffer->ReadableLength() < 1536)
			{
				return SHAKE_DATA_NOT_ENOUGH;
			}
			else
			{
				buffer->AddReadIndex(1536);
				shake_hand_status_ = SHAKE_RTMP_SET_CHUNK_SIZE;
				return SHAKE_RTMP_C2;
			}
		}
		case SHAKE_RTMP_SET_CHUNK_SIZE:
		{
			RtmpPack set_chunk_size_pack;
			parse = ParseHeaderAndBody(buffer, &set_chunk_size_pack);
			if (parse > 0)
			{
				shake_hand_status_ = SHAKE_RTMP_CONNECT;
				return SHAKE_RTMP_SET_CHUNK_SIZE;
			}
			break;
		}
		case SHAKE_RTMP_CONNECT:
		{
			parse = ParseHeaderAndBody(buffer, &connect_pack_);
			if (parse > 0)
			{
				shake_hand_status_ = SHAKE_RTMP_RELEASE_STREAM;
				return SHAKE_RTMP_CONNECT;
			}
			break;
		}
		case SHAKE_RTMP_RELEASE_STREAM:
		{
			parse = ParseHeaderAndBody(buffer, &release_pack_);
			if (parse > 0)
			{
				shake_hand_status_ = SHAKE_RTMP_FCPUBLISH;
				return SHAKE_RTMP_RELEASE_STREAM;
			}
			break;
		}
		case SHAKE_RTMP_FCPUBLISH:
		{
			RtmpPack fc_publish;
			parse = ParseHeaderAndBody(buffer, &fc_publish);
			if (parse > 0)
			{
				shake_hand_status_ = SHAKE_RTMP_CREATE_STREAM;
				return SHAKE_RTMP_FCPUBLISH;
			}
			break;
		}
		case SHAKE_RTMP_CREATE_STREAM:
		{
			if (buffer->ReadableLength() > 0)
			{
				/**
				 * 这里会出现c3 包长度固定 但是上一个包已经写满 暂且不知道原因
				 */
				if ((uint8_t)*buffer->ReadBegin() == 0xc3)
				{
					buffer->AddReadIndex(26);
					shake_hand_status_ = SHAKE_RTMP_PUBLISH;
					return SHAKE_RTMP_CREATE_STREAM;
				}
				else
				{
					RtmpPack create_stream;
					parse = ParseHeaderAndBody(buffer, &create_stream);
					if (parse > 0)
					{
						shake_hand_status_ = SHAKE_RTMP_PUBLISH;
						return SHAKE_RTMP_CREATE_STREAM;
					}
				}
			}
			else
			{
				parse = 0;
			}
			break;
		}
		case SHAKE_RTMP_PUBLISH:
		{
			RtmpPack publish_pack;
			parse = ParseHeaderAndBody(buffer, &publish_pack);
			if (parse > 0)
			{
				shake_hand_status_ = SHAKE_SUCCESS;
				return SHAKE_RTMP_PUBLISH;
			}
			break;
		}
		case SHAKE_FAILED:
			return SHAKE_FAILED;
		case SHAKE_SUCCESS:
			return SHAKE_SUCCESS;
		case SHAKE_DATA_NOT_ENOUGH:
			return SHAKE_DATA_NOT_ENOUGH;
	}

	if (parse == 0)
	{
		return SHAKE_DATA_NOT_ENOUGH;
	}
	else
	{
		shake_hand_status_ = SHAKE_FAILED;
		return SHAKE_FAILED;
	}
}

ssize_t RtmpManager::ParseHeaderAndBody(Buffer* buffer, RtmpPack* rtmp_pack)
{
	ssize_t parse = ParseHeader(buffer, rtmp_pack);
	if (parse > 0)
	{
		size_t need_length = rtmp_pack->GetBodyDataSize() + parse;
		if (need_length > buffer->ReadableLength())
		{
			return 0;
		}
		/**
		 * 长度足够解析body 移动指针
		 */
		buffer->AddReadIndex(parse);
		ParseBody(buffer, rtmp_pack, true, nullptr, nullptr);
		return need_length;
	}
	else
	{
		return parse;
	}
}

size_t RtmpManager::GetParsedLength() const
{
	return parsed_length_;
}
