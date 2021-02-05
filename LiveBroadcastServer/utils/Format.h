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
	 * 将十进制数字 转换为十六进制样式的字符串 同时尾部附加 CRLF
	 * @param decimal_num 如10
	 * @return
	 */
	std::string ToHexStringWithCrlf(size_t decimal_num);

	/**
	 * GET /1111/1111/ HTTP1.1 TO 1111/1111
	 * GET /1111/1111 HTTP1.1 TO 1111/1111
	 * @param str
	 * @return
	 */
	std::string GetUrl(const std::string& str);

	/**
	 * rtmp://example.com:4000/123/ -> 123
	 * rtmp://example.com:4000/123/123 -> 123/123
	 * rtmp://example.com:4000/123/123/ -> 123/123
	 * @param url
	 * @return
	 */
	std::string GetPathFromUrl(const std::string& url);
}
#endif //LIVEBROADCASTSERVER_FORMAT_H
