#ifndef UTILS_BUFFER
#define UTILS_BUFFER

#include <vector>

constexpr int DEFAULT_BUFFER_SIZE = 4096;
constexpr int IDX_BEGIN = 8;

/**
 * 通用缓冲区类
*/
class Buffer
{
public:
	Buffer();
	Buffer(size_t buffer_size);
	~Buffer();

	size_t ReadableLength() const;
	size_t WritableLength() const;

	void AddWriteIndex(size_t index);
	void AddReadIndex(size_t index);

	void AddReadIndexAndAdjust(size_t index);

	char* WriteBegin();

	char* ReadBegin();

	void AdjustBuffer();

	void Reset();

	size_t GetSumRead() const;
	size_t GetSumWrite() const;
private:

	std::vector<char> buffer_;
	size_t read_idx_;
	size_t write_idx_;
	size_t sum_read_;
	size_t sum_write_;
};


#endif // !UTILS_BUFFER