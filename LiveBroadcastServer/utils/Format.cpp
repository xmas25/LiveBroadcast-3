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

std::string Format::ToHexStringWithRN(size_t decimal_num)
{
	return ToHexString(decimal_num) + "\r\n";
}
