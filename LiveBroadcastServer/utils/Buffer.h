#ifndef UTILS_BUFFER
#define UTILS_BUFFER

#include <vector>

constexpr int DEFAULT_BUFFER_SIZE = 4096;
constexpr int IDX_BEGIN = 8;

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

private:

	std::vector<char> buffer_;
	size_t read_idx_;
	size_t write_idx_;
};


#endif // !UTILS_BUFFER