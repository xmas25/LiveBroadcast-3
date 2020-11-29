#include <utils/codec/RtmpManager.h>
#include <cassert>
#include <string>

std::string ROOT = R"(C:\Users\rjd67\Desktop\Server\)";
std::string DATA_FILE = ROOT + "1606023960.rtmp";
std::string OUT_FILE = ROOT + "1606023960.rtmp.flv";

int main()
{
	/* rtmp数据流*/
	File file(DATA_FILE);

	RtmpManager manager;
	Buffer buffer(8192);

	ssize_t result = 0;

	while (result != -1)
	{
		if (file.Read(&buffer) == 0)
		{
			break;
		}

		result = manager.ParseData(&buffer);
	}

	printf("sum_read: %zu, sum_write: %zu\n", buffer.GetSumRead(), buffer.GetSumWrite());

	buffer.Reset();

	FlvManager* flv_manager = manager.GetFlvManager();

	flv_manager->EncodeHeadersToBuffer(&buffer);

	File file_write(OUT_FILE, File::O_WRONLY);
	file_write.Write(buffer.ReadBegin(), buffer.ReadableLength());

	std::vector<FlvTag*>* flv_tags = flv_manager->GetFlvTags();

	ssize_t write_bytes;
	for (FlvTag* tag : *flv_tags)
	{
		write_bytes = file_write.Write(tag->GetHeader(), FlvTag::FLV_TAG_HEADER_LENGTH);
		assert(write_bytes == FlvTag::FLV_TAG_HEADER_LENGTH);

		const Buffer* body = tag->GetBody();
		write_bytes = file_write.Write(body);
		assert(write_bytes == static_cast<ssize_t>(body->ReadableLength()));
	}

	return 0;
}