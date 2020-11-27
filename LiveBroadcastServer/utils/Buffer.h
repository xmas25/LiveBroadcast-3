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

	char* WriteBegin();

	char* ReadBegin();

	void AdjustBuffer();

	/**
	 * 重置Buffer 清除所有数据 重置总读写字节数
	 */
	void Reset();

	size_t GetSumRead() const;
	size_t GetSumWrite() const;

	size_t AppendData(const char* data, size_t length);

	size_t AppendData(const std::string* data);

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
private:

	std::vector<char> buffer_;
	size_t read_idx_;
	size_t write_idx_;
	size_t sum_read_;
	size_t sum_write_;
};


#endif // !UTILS_BUFFER_H