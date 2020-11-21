#include <utils/codec/RtmpManager.h>
#include <cassert>

int main()
{
	/* rtmp数据流*/
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

	buffer.Reset();

	FlvManager* flv_manager = manager.GetFlvManager();

	flv_manager->EncodeHeadersToBuffer(&buffer);

	File file_write("/root/server/2.data", File::O_WRONLY);
	file_write.Write(buffer.ReadBegin(), buffer.ReadableLength());

	std::vector<FlvTag*>* flv_tags = flv_manager->GetFlvTags();

	ssize_t write_bytes;
	for (FlvTag* tag : *flv_tags)
	{
		write_bytes = file_write.Write(tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
		assert(write_bytes == FlvTag::FLV_TAG_HEADER_LENGTH);

		const std::string* body = tag->GetBody()->GetBody();
		write_bytes = file_write.Write(body->data(), body->length());
		assert(write_bytes == static_cast<ssize_t>(body->length()));
	}

	int a;
	scanf("%d", &a);

	return 0;
}