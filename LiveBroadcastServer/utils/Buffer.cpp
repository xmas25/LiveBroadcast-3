#include <cstring>
#include "utils/Buffer.h"


Buffer::Buffer() :
	Buffer(DEFAULT_BUFFER_SIZE)
{
}

Buffer::Buffer(size_t buffer_size) :
	buffer_(buffer_size + IDX_BEGIN),
	read_idx_(IDX_BEGIN),
	write_idx_(IDX_BEGIN),
	sum_read_(0),
	sum_write_(0)
{
}

Buffer::~Buffer()
{
}

size_t Buffer::ReadableLength() const
{
	return write_idx_ - read_idx_;
}

size_t Buffer::WritableLength() const
{
	return buffer_.size() - write_idx_;
}

void Buffer::AddWriteIndex(size_t index)
{
	sum_write_ += index;
	write_idx_ += index;
}

void Buffer::AddReadIndex(size_t index)
{
	if (index > ReadableLength())
	{
		sum_read_ += ReadableLength();
		read_idx_ += ReadableLength();
	}else
	{
		sum_read_ += index;
		read_idx_ += index;
	}

}

char* Buffer::WriteBegin()
{
	return &buffer_[write_idx_];
}

const char* Buffer::ReadBegin() const
{
	return &buffer_[read_idx_];
}

void Buffer::AdjustBuffer()
{
	size_t readable_length = ReadableLength();

	std::copy(&buffer_[read_idx_], &buffer_[write_idx_], &buffer_[IDX_BEGIN]);

	read_idx_ = IDX_BEGIN;
	write_idx_ = IDX_BEGIN + readable_length;
}

void Buffer::Reset()
{
	read_idx_ = IDX_BEGIN;
	write_idx_ = IDX_BEGIN;
	sum_read_ = 0;
	sum_write_ = 0;
}

size_t Buffer::GetSumRead() const
{
	return sum_read_;
}

size_t Buffer::GetSumWrite() const
{
	return sum_write_;
}

size_t Buffer::AppendData(const char* data, size_t length)
{
	AdjustBuffer();
	if (length > WritableLength())
	{
		size_t remain = length - WritableLength();
		ReSize(buffer_.size() + remain);
	}

	memcpy(WriteBegin(), data, length);
	AddWriteIndex(length);
	return length;
}

size_t Buffer::AppendData(const std::string& data)
{
	return AppendData(data.data(), data.length());
}

ssize_t Buffer::ReadFromSockfd(SOCKET sockfd)
{
	AdjustBuffer();
	ssize_t result = recv(sockfd, WriteBegin(), WritableLength(), 0);
	if (result <= 0)
	{
		return result;
	}
	AddWriteIndex(result);
	return result;
}

ssize_t Buffer::ReadFromSockfdAndDrop(SOCKET sockfd)
{
	AdjustBuffer();
	ssize_t result = recv(sockfd, WriteBegin(), WritableLength(), 0);
	return result;
}

std::string Buffer::ReadAllAsString()
{
	std::string result(ReadBegin(), ReadableLength());
	AddReadIndex(ReadableLength());
	return result;
}

void Buffer::ReSize(size_t new_size)
{
	buffer_.resize(new_size + IDX_BEGIN);
}

void Buffer::SwapBuffer(Buffer* buffer)
{
	if (!buffer)
	{
		return;
	}

	buffer_.swap(buffer->buffer_);
	Swap(read_idx_, buffer->read_idx_);
	Swap(write_idx_, buffer->write_idx_);
	Swap(sum_write_, buffer->sum_write_);
	Swap(sum_read_, buffer->sum_read_);
}

void Buffer::Swap(size_t& lhs, size_t& rhs)
{
	size_t temp = rhs;
	rhs = lhs;
	lhs = temp;
}

size_t Buffer::AppendData(const Buffer* buffer)
{
	size_t readable_data = buffer->ReadableLength();
	AppendData(buffer->ReadBegin(), readable_data);
	return readable_data;
}

void Buffer::DropAllData()
{
	AddReadIndex(ReadableLength());
}
