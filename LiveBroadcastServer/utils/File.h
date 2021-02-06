#ifndef UTILS_FILE_H
#define UTILS_FILE_H

#include <string>
#include <cstdio>
#include <sys/stat.h>

class Buffer;
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

	/**
	 * 打开指定文件 默认只读模式
	 * @param file 文件完整路径
	 */
	File(const std::string& file);

	/**
	 * 打开指定文件
	 * @param file 文件完整路径
	 * @param openmode 打开模式
	 */
	File(const std::string& file, OpenMode openmode);

	~File();

	/**
	 * 按指定模式 打开指定文件
	 * @param file
	 * @param openmode
	 * @return  存在打开的文件则返回false
	 */
	bool Open(const std::string& file, OpenMode openmode = O_RDONLY);

	void Close();

	ssize_t Read(char* buffer, size_t length);

	ssize_t Read(Buffer* buffer);

	ssize_t Write(const char* buffer, size_t length);

	ssize_t Write(const Buffer* buffer);

	size_t GetFileSize();

	void Flush();

	const std::string& GetPath() const;
private:
	std::string path_;

	FILE* file_;

	struct stat stat_;

	std::string OpenModeToString(OpenMode openmode);
};

#endif