#ifndef UTILS_BUFFER_H
#define UTILS_BUFFER_H

#include <vector>
#include <string>

#include "utils/PlatformBase.h"

constexpr int DEFAULT_BUFFER_SIZE = 4096;
constexpr int IDX_BEGIN = 8;

/**
 * 通用缓冲区
*/
class Buffer
{
public:
	Buffer();
	explicit Buffer(size_t buffer_size);
	~Buffer();

	size_t ReadableLength() const;
	size_t WritableLength() const;

	void AddWriteIndex(size_t index);
	void AddReadIndex(size_t index);

	/**
	 * 丢弃所有未读数据
	 */
	void DropAllData();

	char* WriteBegin();

	const char* ReadBegin() const;

	void AdjustBuffer();

	/**
	 * 重置Buffer 清除所有数据 重置总读写字节数
	 */
	void Reset();

	void ReSize(size_t new_size);

	size_t GetSumRead() const;
	size_t GetSumWrite() const;

	size_t AppendData(const char* data, size_t length);

	size_t AppendData(const std::string& data);

	size_t AppendData(const Buffer* buffer);

	std::string ReadAllAsString();
	/**
	 * 从sockfd中读取数据并保存到Buffer中
	 * @param sockfd
	 * @return -1 出错 0 结束 大于0为读取的长度
	 */
	ssize_t ReadFromSockfd(SOCKET sockfd);

	/**
	 * 从sockfd中读取数据并丢弃
	 * @param sockfd
	 * @return 丢弃的数据长度
	 */
	ssize_t ReadFromSockfdAndDrop(SOCKET sockfd);

	void SwapBuffer(Buffer* buffer);

protected:

	std::vector<char> buffer_;
	size_t read_idx_;
	size_t write_idx_;
	size_t sum_read_;
	size_t sum_write_;

private:
	void Swap(size_t& lhs, size_t& rhs);
};


#endif // !UTILS_BUFFER_H