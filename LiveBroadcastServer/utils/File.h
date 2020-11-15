#ifndef UTILS_FILE_H
#define UTILS_FILE_H

#include <string>
#include <cstdio>
#include <sys/stat.h>

class File
{
public:

	enum OpenMode
	{
		O_RDONLY,
		O_WRONLY,
		O_APPEND,
		O_RDWR,
	};

	File();
	File(const std::string& file);
	File(const std::string& file, OpenMode openmode);

	~File();

	bool Open(const std::string& file, OpenMode openmode = O_RDONLY);

	void Close();

	ssize_t Read(char* buffer, size_t length);

	ssize_t Write(char* buffer, size_t length);

	size_t GetFileSize();

private:
	std::string path_;

	FILE* file_;

	struct stat stat_;

	std::string OpenModeToString(OpenMode openmode);
};

#endif