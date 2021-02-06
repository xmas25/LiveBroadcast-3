#include <sstream>
#include "utils/codec/RtmpCodec.h"
#include "utils/codec/FlvCodec.h"

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

void RtmpCodec::EncodeHeaderToFlvTag(RtmpPack* rtmp_pack_, FlvTag* flv_tag)
{
	flv_tag->SetTagType(static_cast<uint8_t>(rtmp_pack_->GetRtmpPackType()));
	flv_tag->SetDataSize(rtmp_pack_->GetDataSizePtr());

	AddTimeStamp(rtmp_pack_->GetTimeStamp());
	uint8_t ts[3] = {ts_[2], ts_[1], ts_[0]};
	flv_tag->SetTimeStamp(ts);
}

RtmpCodec::RtmpCodec() :
	timestamp_(0)
{

}

void RtmpCodec::AddTimeStamp(const uint8_t* timestamp)
{
	timestamp_ += timestamp[2];
	timestamp_ += timestamp[1] * 256;
	timestamp_ += timestamp[0] * 65536;
}

void RtmpCodec::EncodeHeaderAndSwapBuffer(RtmpPack* rtmp_pack_, FlvTag* flv_tag)
{
	EncodeHeaderToFlvTag(rtmp_pack_, flv_tag);
	SwapBuffer(rtmp_pack_, flv_tag);
}

void RtmpCodec::SwapBuffer(RtmpPack* rtmp_pack_, FlvTag* flv_tag)
{
	rtmp_pack_->GetBuffer()->
		SwapBuffer(flv_tag->GetBody());
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

	ssize_t result;
	switch (fmt_)
	{
		case FMT0:
			result = DecodeFmt0(data + 1, length - 1);
			break;
		case FMT1:
			result = DecodeFmt1(data + 1, length - 1);
			break;
		case FMT2:
			result = DecodeFmt2(data + 1, length - 1);
			break;
		case FMT3:
			result = FMT3_HEADER_LENGTH;
			return result + 1; /* 不直接返回会与下面逻辑混淆*/
		default:
			result = -1;
			break;
	}

	if (result <= 0)
	{
		return result;
	}
	else
	{
		return result + 1;// 2b fmt and 6b csid_
	}
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
	switch (type)
	{
		case 8:
		case 9:
		case 18:
		case 2:
		case 3:
		case 4:
			pack_type_ = static_cast<RtmpPackType>(type);
			break;
		default:
			pack_type_ = RtmpPackType::RTMP_OTHER;
	}
}

uint32_t RtmpPack::GetBodyDataSize() const
{
	/*
	 *data_size为三个字节的十六进制数据
	*/
	return data_size_[0] * 65536 + data_size_[1] * 256 + data_size_[2];
}

void RtmpPack::SetBodyDataSize(uint32_t data_size)
{
	uint8_t* data_size_ptr =reinterpret_cast<uint8_t*>(&data_size);
	data_size_[0] = data_size_ptr[2];
	data_size_[1] = data_size_ptr[1];
	data_size_[2] = data_size_ptr[0];
}

const uint8_t* RtmpPack::GetDataSizePtr() const
{
	return data_size_;
}

RtmpPack::RtmpPackFmt RtmpPack::GetFmt() const
{
	return fmt_;
}

uint8_t RtmpPack::GetCsid() const
{
	return csid_;
}

const uint8_t* RtmpPack::GetTimeStamp() const
{
	return timestamp_;
}

uint32_t RtmpPack::GetBodyRemainSize() const
{
	return GetBodyDataSize() - GetBodyCurrentSize();
}

uint32_t RtmpPack::GetBodyCurrentSize() const
{
	return buffer_.ReadableLength();
}

void RtmpPack::AppendData(const char* data, size_t length)
{
	buffer_.AppendData(data, length);
}

Buffer* RtmpPack::GetBuffer()
{
	return &buffer_;
}

std::string RtmpPack::GetHeaderDebugMessage()
{
	std::ostringstream ss;

	uint8_t fmt = (static_cast<uint8_t>(fmt_) << 6) + csid_;
	ss << std::hex << (int)fmt << " ";


	switch (fmt_)
	{
		case FMT0:
			ss << (int)timestamp_[0] << " " << (int)timestamp_[1] << " " << (int)timestamp_[2] << " ";
			ss << (int)data_size_[0] << " " << (int)data_size_[1] << " " << (int)data_size_[2] << " ";
			ss << static_cast<int>(pack_type_);
			break;
		case FMT1:
			ss << (int)timestamp_[0] << " " << (int)timestamp_[1] << " " << (int)timestamp_[2] << " ";
			ss << (int)data_size_[0] << " " << (int)data_size_[1] << " " << (int)data_size_[2] << " ";
			ss << static_cast<int>(pack_type_);
			break;
		case FMT2:
			ss << (int)timestamp_[0] << " " << (int)timestamp_[1] << " " << (int)timestamp_[2] << " ";
			break;
		case FMT3:
			break;
	}
	return ss.str();
}
