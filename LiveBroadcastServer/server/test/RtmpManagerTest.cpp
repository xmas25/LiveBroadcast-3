#include <server/RtmpManager.h>
#include <cassert>

int main()
{
	/**
	 * 解析Rtmp数据流测试
	 * 
	 * 1.data 是完整保存的rtmp数据流
	*/
	File file("/root/server/1.data");

	RtmpManager manager;
	Buffer buffer(8192);

	ssize_t result = 0;

	while (result != -1)
	{
		file.Read(&buffer);
		result = manager.ParseData(&buffer);
	}
	
	printf("sum_read: %zu, sum_write: %zu\n", buffer.GetSumRead(), buffer.GetSumWrite());

	/**
	* 写入文件测试
	*/
	buffer.Reset();

	FlvManager* flv_manager = manager.GetFlvManager();

	flv_manager->EncodeHeadersToBuffer(&buffer);

	File file_write("/root/server/2.data", File::O_WRONLY);
	file_write.Write(buffer.ReadBegin(), buffer.ReadableLength());

	std::vector<FlvTag*>* flv_tags = flv_manager->GetFlvTags();

	ssize_t write_bytes;
	for (FlvTag* tag : *flv_tags)
	{
		FlvTagZeroCopy* copy = tag->GetZeroCopyCache();
		const char* header = copy->GetHeader();
		write_bytes = file_write.Write(header, FlvTag::FLV_TAG_HEADER_LENGTH);
		assert(write_bytes == FlvTag::FLV_TAG_HEADER_LENGTH);

		const std::string* body = copy->GetBody();
		write_bytes = file_write.Write(body->data(), body->length());
		assert(write_bytes == static_cast<ssize_t>(body->length()));
	}

	int a;
	scanf("%d", &a);

	return 0;
}