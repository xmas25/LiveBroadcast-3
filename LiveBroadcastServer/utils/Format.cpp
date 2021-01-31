//
// Created by rjd67 on 2020/11/29.
//

#include <sstream>
#include "utils/Format.h"

std::string Format::ToHexString(size_t decimal_num)
{
	std::stringstream stream;
	stream << std::hex << decimal_num;
	return stream.str();
}

std::string Format::ToHexStringWithCrlf(size_t decimal_num)
{
	return ToHexString(decimal_num) + "\r\n";
}

std::string Format::GetUrl(const std::string& str)
{
	auto url_begin = str.find(' ') + 1;
	auto url_end = str.find(' ', url_begin);
	if (str[url_end - 1] == '/')
	{
		url_end--;
	}
	return str.substr(url_begin + 1, url_end - url_begin - 1);
}

std::string Format::GetPathFromUrl(const std::string& url)
{
	auto idx = url.find("//");
	auto idx1 = url.find('/', idx + 2);

	std::string path = url.substr(idx1);

	size_t path_len = path.length();
	if (path.find_last_of('/') == path.length() - 1)
	{
		path_len--;
	}
	return path.substr(1, path_len - 1);
}
