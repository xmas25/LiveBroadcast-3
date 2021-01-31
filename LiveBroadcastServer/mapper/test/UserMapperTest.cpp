//
// Created by rjd67 on 2021/1/31.
//
#include <mapper/UserMapper.h>

int main()
{
	UserMapper mapper;

	mapper.Initialize("192.168.80.160", "lsmg", "123456789", "live_test");

	std::string user = "ww";
	std::string passwd = mapper.GetPasswdByUser(user);

	printf("user: %s, passwd: %s\n", user.c_str(), passwd.c_str());
}