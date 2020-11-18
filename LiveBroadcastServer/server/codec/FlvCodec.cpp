#include <netinet/in.h>
#include "server/codec/FlvCodec.h"

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


FlvTagZeroCopy::FlvTagZeroCopy(const std::string* data) :
    body_(data)
{
}

void FlvTagZeroCopy::EncodeHeader(uint32_t previous_tag_size, uint8_t tag_type, uint8_t* data_size, uint8_t* timestamp,
    uint8_t timestamp_extend, uint8_t* stream_id)
{
    memcpy(&header_[0], &previous_tag_size, sizeof previous_tag_size);
    memcpy(&header_[4], &tag_type, 1);
    memcpy(&header_[5], data_size, 3);
    memcpy(&header_[8], timestamp, 3);
    memcpy(&header_[11], &timestamp_extend, 1);
    memcpy(&header_[12], stream_id, 3);
}

ssize_t FlvTagZeroCopy::CopyToBuffer(char* buffer, size_t buffer_length)
{
    size_t data_len = FlvTag::FLV_TAG_HEADER_LENGTH + body_->length();
    if (buffer_length < data_len)
    {
        return 0;
    }

    memcpy(buffer, header_, FlvTag::FLV_TAG_HEADER_LENGTH);
    memcpy(buffer + FlvTag::FLV_TAG_HEADER_LENGTH, body_->data(), body_->length());
    return data_len;
}

const char* FlvTagZeroCopy::GetHeader() const
{
    return header_;
}

const std::string* FlvTagZeroCopy::GetBody() const
{
    return body_;
}

FlvTagZeroCopy* FlvTag::GetZeroCopyCache()
{
    if (!encode_)
    {
        cache_.EncodeHeader(previous_tag_size_, tag_type_, data_size_, timestamp_,
            timestamp_extend_, stream_id_);

        encode_ = true;
    }
    return &cache_;
}

FlvTag::FlvTag() :
    cache_(&body_),
    encode_(false)
{
    Init();
}

FlvTag::~FlvTag()
{
    
}

uint32_t FlvTag::GetDataSize() const
{
    /*
     *data_size为三个字节的十六进制数据
    */
    return data_size_[0] * 65536 + data_size_[1] * 256 + data_size_[2];
}

uint32_t FlvTag::GetPreviousTagSize() const
{
    /*
    previous_tag_size_为大端序
    */
    return ntohl(previous_tag_size_);
}

uint32_t FlvTag::GetTagSize() const
{   /*长度部分 除去previous_tag_size_ 附加data长度*/
    return FLV_TAG_HEADER_LENGTH - 4 + GetDataSize();
}

uint32_t FlvTag::GetCurrentDataSize() const
{
    return body_.length();
}

uint32_t FlvTag::GetRemainDataSize() const
{
    return GetDataSize() - GetCurrentDataSize();
}

void FlvTag::SetData(const char* data, size_t length)
{
    if (length >= GetDataSize())
    {
        if (!body_.empty())
        {
            body_.clear();
        }
        body_.append(data, GetDataSize());
    }
}

void FlvTag::AppendData(const char* data, size_t length)
{
    body_.append(data, length);
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


void FlvTag::Init()
{
    previous_tag_size_ = 0;
    tag_type_ = 0;
    memset(&data_size_, 0, sizeof data_size_);
    memset(&timestamp_, 0, sizeof timestamp_);
    timestamp_extend_ = 0;
    memset(&stream_id_, 0, sizeof stream_id_);
    body_.clear();
}

void FlvTag::SetTagType(uint8_t tag_type)
{
    tag_type_ = tag_type;
}

void FlvTag::SetDataSize(uint8_t* data_size)
{
    memcpy(data_size_, data_size, sizeof data_size_);
}

void FlvTag::SetTimeStamp(uint8_t* timestamp)
{
    memcpy(timestamp_, timestamp, sizeof timestamp_);
}

void FlvTag::SetSteamId(uint8_t* stream_id)
{
    memcpy(stream_id_, stream_id, sizeof stream_id_);
}

void FlvTag::SetPreviousTagSize(uint32_t previous_tag_size)
{
    // 统一使用大端序存储数据 便于统一序列化
    previous_tag_size_ = htons(previous_tag_size);
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