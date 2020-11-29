//
// Created by rjd67 on 2020/11/25.
//

#include "PlatformNetwork.h"

#ifdef _WIN32
NetworkInitializer::NetworkInitializer()
{
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	::WSAStartup(version, &data);
}

NetworkInitializer::~NetworkInitializer()
{
	::WSACleanup();
}
#endif

std::string GetLastErrorAsString()
{
#ifdef _WIN32
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if(errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);
	return message;
#else
	return strerror(errno);
#endif
}


int write(SOCKET s, const uint8_t* buf, int len)
{
	return send(s, reinterpret_cast<const char*>(buf), len, 0);
}

int read(SOCKET s, char* buf, int len)
{
	return recv(s, buf, len, 0);
}