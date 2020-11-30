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
	auto url_end = str.find(url_begin, ' ');
	return str.substr(url_begin, url_end - url_begin);
}
