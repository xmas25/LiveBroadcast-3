//
// Created by rjd67 on 2020/12/27.
//

#ifndef LIVEBROADCASTSERVER_DBMYSQL_H
#define LIVEBROADCASTSERVER_DBMYSQL_H

#include <mysql/mysql.h>
#include <string>

#include "QueryResult.h"

class DbMysql
{
public:
	struct DatabaseInfo
	{
		std::string host;
		std::string user;
		std::string passwd;
		std::string db_name;
	};

public:

	DbMysql();
	~DbMysql();

	bool Initialize(const std::string& host, const std::string& user, const std::string& passwd,
			const std::string& db_name);

	/**
	 * 执行查询语句
	 * @param sql 语句命令
	 * @return
	 */
	QueryResultPtr Query(const char* sql);

	/**
	 * 通用执行语句 sql语句中不能包含二进制数据
	 * @param sql
	 * @return
	 */
	bool Execute(const char* sql);
private:

	DatabaseInfo db_info_;

	bool inited_;
	MYSQL* mysql_;

};


#endif //LIVEBROADCASTSERVER_DBMYSQL_H
