//
// Created by rjd67 on 2020/12/27.
//
#include <cstdio>
#include "../DbMysql.h"
#include "utils/Logger.h"

int main()
{
	DbMysql mysql;

	bool init_result = mysql.Initialize("localhost", "root", "123456789", "db_test");
	printf("Init %s\n", init_result ? "success" : "fail");

	QueryResultPtr result = mysql.Query("SELECT username, password FROM user");
	do
	{
		LOG_INFO("username: %s, password: %s", (*result)["username"].GetCStr(), (*result)["password"].GetCStr());
	} while(result->NextRow());

	std::string username = "insert-k";
	std::string password = "insert-v";

	char sql[256];
	snprintf(sql, sizeof sql, "INSERT INTO user(username, password) VALUES('%s', '%s')",
			username.c_str(), password.c_str());
	bool insert_result = mysql.Execute(sql);
	if (!insert_result)
	{
		LOG_ERROR("Insert error");
	}

	return 0;
}