//
// Created by rjd67 on 2020/11/29.
//

#ifndef LIVEBROADCASTSERVER_FORMAT_H
#define LIVEBROADCASTSERVER_FORMAT_H

#include <string>
namespace Format
{
	/**
	 * 将十进制数字 转换为十六进制样式的字符串
	 * @param decimal_num 如10
	 * @return "a"
	 */
	std::string ToHexString(size_t decimal_num);

	/**
	 * 将十进制数字 转换为十六进制样式的字符串 同时尾部附加 \r\n
	 * @param decimal_num 如10
	 * @return "a\r\n"
	 */
	std::string ToHexStringWithCrlf(size_t decimal_num);

	std::string GetUrl(const std::string& str);
}
#endif //LIVEBROADCASTSERVER_FORMAT_H
