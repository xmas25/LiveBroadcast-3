//
// Created by rjd67 on 2021/1/31.
//

#ifndef LIVEBROADCASTSERVER_USERMAPPER_H
#define LIVEBROADCASTSERVER_USERMAPPER_H

#include "mysql/DbMysql.h"

class UserMapper
{
public:
	UserMapper() = default;

	bool Initialize(const std::string& host, const std::string& user, const std::string& passwd,
			const std::string& db_name);

	std::string GetPasswdByUser(const std::string& username);

private:

	DbMysql mysql_;
};


#endif //LIVEBROADCASTSERVER_USERMAPPER_H
