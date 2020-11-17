#include <cstring>
#include "Buffer.h"


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
	sum_read_ += index;
	read_idx_ += index;
}

void Buffer::AddReadIndexAndAdjust(size_t index)
{
	AddReadIndex(index);
	AdjustBuffer();
}

char* Buffer::WriteBegin()
{
	return &buffer_[write_idx_];
}

char* Buffer::ReadBegin()
{
	return &buffer_[read_idx_];
}

void Buffer::AdjustBuffer()
{
	size_t readable_length = ReadableLength();
	if (readable_length == 0)
	{
		Reset();
		return;
	}

	memcpy(&buffer_[IDX_BEGIN], &buffer_[read_idx_], readable_length);
	read_idx_ = IDX_BEGIN;
	write_idx_ = IDX_BEGIN + readable_length;
}

void Buffer::Reset()
{
	read_idx_ = IDX_BEGIN;
	write_idx_ = IDX_BEGIN;
}

size_t Buffer::GetSumRead() const
{
	return sum_read_;
}

size_t Buffer::GetSumWrite() const
{
	return sum_write_;
}
