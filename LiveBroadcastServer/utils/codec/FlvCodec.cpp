#include "utils/codec/FlvCodec.h"

char FlvHeader::DEFAULT_HEADER[] = { 0x46, 0x4C, 0x56, 0x01, 0x05, 0x00, 0x00, 0x00, 0x09 };

ssize_t FlvCodec::DecodeFileHeader(const char* data, size_t length, FlvHeader* tag)
{
	if (length < FlvHeader::FLV_HEADER_LENGTH)
	{
		return 0;
	}

	std::string flv;
	flv.append(&data[0], 3);

	uint8_t version, type_flag;
	uint32_t header_length;

	memcpy(&version, &data[3], 1);
	memcpy(&type_flag, &data[4], 1);
	memcpy(&header_length, &data[5], 4);

	if (tag)
	{
		tag->SetInfo(flv, version, type_flag, header_length);
	}

	return FlvHeader::FLV_HEADER_LENGTH;
}

ssize_t FlvCodec::DecodeTagHander(const char* data, size_t length, FlvTag* tag)
{
	if (length < FlvTag::FLV_TAG_HEADER_LENGTH || !tag)
	{
		return 0;
	}

	return tag->DecodeTagHander(data, length);
}

FlvTag::FlvTag() :
		header_()
{

}

FlvTag::~FlvTag()
{

}

uint32_t FlvTag::GetDataSize() const
{
	/*
	 *data_size为三个字节的十六进制数据 注意转换为uint8_t否则会出错
	*/

	return static_cast<uint8_t>(header_[DATA_SIZE_SUB]) * 65536 +
		   static_cast<uint8_t>(header_[DATA_SIZE_SUB + 1]) * 256 +
		   static_cast<uint8_t>(header_[DATA_SIZE_SUB + 2]);
}

uint32_t FlvTag::GetPreviousTagSize() const
{
	/*
	previous_tag_size_为大端序
	*/
	const uint32_t* previous_tag_size = reinterpret_cast<const uint32_t*>(&header_[PREVIOUS_TAG_SIZE_SUB]);
	return previous_tag_size[0] * 0x1000000+
			previous_tag_size[0] * 0x10000+
			previous_tag_size[0] * 0x100+
			previous_tag_size[0];
}

uint32_t FlvTag::GetCurrentTagSize() const
{   /*长度部分 除去previous_tag_size_ 附加data长度*/
	return FLV_TAG_HEADER_LENGTH - 4 + GetDataSize();
}

uint32_t FlvTag::GetBodyDataLength() const
{
	return body_.ReadableLength();
}

uint32_t FlvTag::GetRemainDataSize() const
{
	return GetDataSize() - GetBodyDataLength();
}

void FlvTag::AppendData(const char* data, size_t length)
{
	body_.AppendData(data, length);
}

ssize_t FlvTag::DecodeTagHander(const char* data, size_t length)
{
	memcpy(header_, data, FLV_TAG_HEADER_LENGTH);

	return FLV_TAG_HEADER_LENGTH;
}

void FlvTag::SetTagType(uint8_t tag_type)
{
	header_[TAG_TYPE_SUB] = tag_type;
}

void FlvTag::SetDataSize(const uint8_t* data_size)
{
	memcpy(&header_[DATA_SIZE_SUB], data_size, DATA_SIZE_LENGTH);
}

void FlvTag::SetTimeStamp(uint8_t* timestamp)
{
	memcpy(&header_[TIMESTAMP_SUB], timestamp, TIMESTAMP_LENGTH);
}

void FlvTag::SetSteamId(uint8_t* stream_id)
{
	memcpy(&header_[STREAM_ID_SUB], stream_id, STREAM_ID_LENGTH);
}


const Buffer* FlvTag::GetBody() const
{
	return &body_;
}

const char* FlvTag::GetHeader() const
{
	return header_;
}

void FlvTag::SetPreviousTagSize(uint32_t previous_tag_size)
{
	// 统一使用大端序存储数据 便于统一序列化
	uint8_t* previous_tag_size_t = reinterpret_cast<uint8_t*>(&previous_tag_size);
	uint8_t* previous_tag_size_t_ = reinterpret_cast<uint8_t*>(&header_[PREVIOUS_TAG_SIZE_SUB]);

	previous_tag_size_t_[0] = previous_tag_size_t[3];
	previous_tag_size_t_[1] = previous_tag_size_t[2];
	previous_tag_size_t_[2] = previous_tag_size_t[1];
	previous_tag_size_t_[3] = previous_tag_size_t[0];
}

uint8_t FlvTag::GetTagType() const
{
	return header_[TAG_TYPE_SUB];
}

Buffer* FlvTag::GetBody()
{
	return &body_;
}

ssize_t FlvHeader::EncodeToBuffer(char* data, size_t length)
{
	if (length < FLV_HEADER_LENGTH)
	{
		return 0;
	}

	memcpy(&data[0], DEFAULT_HEADER, sizeof DEFAULT_HEADER);

	return FLV_HEADER_LENGTH;
}