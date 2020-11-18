#include "RtmpCodec.h"
#include "RtmpCodec.h"
#include "RtmpCodec.h"
#include "RtmpCodec.h"
#include "RtmpCodec.h"
#include "server/codec/RtmpCodec.h"
#include "server/codec/FlvCodec.h"

ssize_t RtmpCodec::DecodeHeader(const char* data, size_t length, RtmpPack* rtmp_pack_)
{
	if (!rtmp_pack_ || !data)
	{
		return -1;
	}

	if (length == 0)
	{
		return 0;
	}

	return rtmp_pack_->DecodeHeader(data, length);
}

ssize_t RtmpPack::DecodeHeader(const char* data, size_t length)
{
	uint8_t fmt = (uint8_t)data[0] >> 6;
	csid_ = data[0] & 0b00111111;

	if (fmt > RTMPPACK_FMT_MAX)
	{
		return -1;
	}

	fmt_ = static_cast<RtmpPackFmt>(fmt);

	ssize_t result = 1;
	switch (fmt_)
	{
	case FMT0:
		result += DecodeFmt0(data + 1, length - 1);
		break;
	case FMT1:
		result += DecodeFmt1(data + 1, length - 1);
		break;
	case FMT2:
		result += DecodeFmt2(data + 1, length - 1);
		break;
	case FMT3:
		result += FMT3_HEADER_LENGTH;
		break;
	default:
		result = -1;
		break;
	}

	return result;
}

bool RtmpPack::EncodeHeaderToFlvTag(FlvTag* flv_tag)
{
	if (!flv_tag)
	{
		return false;
	}

	flv_tag->SetTagType(static_cast<uint8_t>(pack_type_));
	flv_tag->SetDataSize(data_size_);
	flv_tag->SetTimeStamp(timestamp_);

	return true;
}

RtmpPack::RtmpPackType RtmpPack::GetRtmpPackType() const
{
	return pack_type_;
}

ssize_t RtmpPack::DecodeFmt0(const char* data, size_t length)
{
	if (length < FMT0_HEADER_LENGTH)
	{
		return 0;
	}

	DecodeFmt1(data, length);

	memcpy(&stream_id_, &data[7], sizeof stream_id_);

	return FMT0_HEADER_LENGTH;
}

ssize_t RtmpPack::DecodeFmt1(const char* data, size_t length)
{
	if (length < FMT1_HEADER_LENGTH)
	{
		return 0;
	}

	memcpy(timestamp_, &data[0], sizeof timestamp_);
	memcpy(data_size_, &data[3], sizeof data_size_);

	SetPackType(data[6]);

	return FMT1_HEADER_LENGTH;
}

ssize_t RtmpPack::DecodeFmt2(const char* data, size_t length)
{
	if (length < FMT2_HEADER_LENGTH)
	{
		return 0;
	}
	memcpy(timestamp_, &data[0], sizeof timestamp_);

	return FMT2_HEADER_LENGTH;
}

void RtmpPack::SetPackType(uint8_t type)
{
	if (type == 8)
	{
		pack_type_ = RtmpPack::RTMP_AUDIO;
	}
	else if (type == 9)
	{
		pack_type_ = RtmpPack::RTMP_VIDEO;
	}
	else if (type == 18)
	{
		pack_type_ = RtmpPack::RTMP_SCRIPT;
	}
	else
	{
		pack_type_ = RtmpPack::RTMP_OTHER;
	}
}

uint32_t RtmpPack::GetDataSize() const
{
	/*
	 *data_sizeΪ�����ֽڵ�ʮ����������
	*/
	return data_size_[0] * 65536 + data_size_[1] * 256 + data_size_[2];
}

RtmpPack::RtmpPackFmt RtmpPack::GetFmt() const
{
	return fmt_;
}

uint8_t RtmpPack::GetCsid() const
{
	return csid_;
}