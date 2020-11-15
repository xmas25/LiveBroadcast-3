#include <cassert>

#include "utils/File.h"

File::File() :
	path_(),
	file_(nullptr),
	stat_({})
{
}

File::File(const std::string& file):
	File(file, File::RDONLY)
{
}

File::File(const std::string& file, OpenMode openmode):
	path_(file),
	stat_()
{
	file_ = fopen(file.c_str(), OpenModeToString(openmode).c_str());
}

File::~File()
{
	Close();
}

bool File::Open(const std::string& file, OpenMode openmode)
{
	if (file_)
	{
		return false;
	}
	file_ = fopen(file.c_str(), OpenModeToString(openmode).c_str());

	return file_ != nullptr;
}

void File::Close()
{
	if (file_)
	{
		fclose(file_);
		file_ = nullptr;
	}
}

ssize_t File::Read(char* buffer, size_t length)
{
	if (!file_)
	{
		return -1;
	}
	ssize_t result = fread(buffer, 1, length, file_);

	assert(result >= 0);

	return result;
}

size_t File::GetFileSize()
{
	if (stat_.st_size == 0)
	{
		stat(path_.c_str(), &stat_);
	}

	return stat_.st_size;
}

std::string File::OpenModeToString(OpenMode openmode)
{
	std::string mode;
	switch (openmode)
	{
	case File::RDONLY:
		mode = "r";
		break;
	case File::WRONLY:
		mode = "w";
		break;
	case File::APPEND:
		mode = "a";
		break;
	case File::RDWR:
		mode = "r+";
		break;
	default:
		break;
	}
	return mode;
}
