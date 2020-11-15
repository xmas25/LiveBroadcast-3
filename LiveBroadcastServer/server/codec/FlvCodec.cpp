#include <netinet/in.h>
#include "server/codec/FlvCodec.h"

ssize_t FlvCodec::DecodeFileHeader(const char* data, size_t length, FlvHeader* tag)
{
    if (length < FLV_HEADER_LENGTH)
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

    return FLV_HEADER_LENGTH;
}

ssize_t FlvCodec::DecodeTagHander(const char* data, size_t length, FlvTag* tag)
{
    if (length < FLV_TAG_HEADER_LENGTH || !tag)
    {
        return 0;
    }

    return tag->DecodeTagHander(data, length);
}

FlvTag::FlvTag()
{
    Init();
}

FlvTag::~FlvTag()
{
    
}

uint32_t FlvTag::GetDataSize() const
{
    return data_size_[0] * 65536 + data_size_[1] * 256 + data_size_[2];
}

uint32_t FlvTag::GetPreviousTagSize() const
{
    return ntohl(previous_tag_size_);
}

uint32_t FlvTag::GetTagSize() const
{
    return FLV_TAG_HEADER_LENGTH - 4 + GetDataSize();
}

uint32_t FlvTag::GetCurrentDataSize() const
{
    return data_.length();
}

void FlvTag::SetData(const char* data, size_t length)
{
    if (length >= GetDataSize())
    {
        if (!data_.empty())
        {
            data_.clear();
        }
        data_.append(data, GetDataSize());
    }
}

void FlvTag::AppendData(const char* data, size_t length)
{
    data_.append(data, length);
}

ssize_t FlvTag::DecodeTagHander(const char* data, size_t length)
{
    size_t sub = 0;

    memcpy(&previous_tag_size_, &data[sub], 4);
    sub += 4;

    memcpy(&tag_type_, &data[sub], 1);
    sub += 1;

    memcpy(&data_size_, &data[sub], 3);
    sub += 3;

    memcpy(&timestamp_, &data[sub], 3);
    sub += 3;

    memcpy(&timestamp_extend_, &data[sub], 1);
    sub += 1;

    memcpy(&stream_id_, &data[sub], 3);
    sub += 3;

    return sub;
}

size_t FlvTag::EncodeTag(const char* buffer, size_t length)
{
    if (length)
}

void FlvTag::Init()
{
    previous_tag_size_ = 0;
    tag_type_ = 0;
    memset(&data_size_, 0, sizeof data_size_);
    memset(&timestamp_, 0, sizeof timestamp_);
    timestamp_extend_ = 0;
    memset(&stream_id_, 0, sizeof stream_id_);
}
