#include <server/RtmpManager.h>


int main()
{
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

	int a;
	scanf("%d", &a);

	return 0;
}