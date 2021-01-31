//
// Created by rjd67 on 2020/12/27.
//
#include <cstring>
#include <mysql/errmsg.h>
#include "mysql/DbMysql.h"
#include "utils/Logger.h"

DbMysql::DbMysql():
	inited_(false),
	mysql_(nullptr)
{
}

DbMysql::~DbMysql()
{
	if (mysql_)
	{
		if (inited_)
		{
			mysql_close(mysql_);
		}
	}
}

bool DbMysql::Initialize(const std::string& host, const std::string& user, const std::string& passwd,
		const std::string& db_name)
{
	if (inited_)
	{
		mysql_close(mysql_);
	}

	db_info_.host = host;
	db_info_.user = user;
	db_info_.passwd = passwd;
	db_info_.db_name = db_name;

	mysql_ = mysql_init(mysql_);

	/** port 0 使用默认端口*/
	MYSQL* result = mysql_real_connect(mysql_, host.c_str(), user.c_str(), passwd.c_str(),
			db_name.c_str(), 0, nullptr, 0);

	if (result)
	{
		inited_ = true;
		mysql_query(mysql_, "set names utf8");

		return true;
	}
	else
	{
		LOG_ERROR("Failed to connect to database: Error: %s",
				mysql_error(mysql_));

		mysql_close(mysql_);
		return false;
	}

	return false;
}

QueryResultPtr DbMysql::Query(const char* sql)
{
	if (!inited_)
	{
		if (!Initialize(db_info_.host, db_info_.user, db_info_.passwd, db_info_.db_name))
		{
			return nullptr;
		}
	}

	int query_result = mysql_real_query(mysql_, sql, strlen(sql));

	if (query_result == 0)
	{
		MYSQL_RES* result = mysql_store_result(mysql_);
		uint32_t num_fields;
		uint64_t num_row;
		if (result)
		{
			num_fields = mysql_field_count(mysql_);
			num_row = mysql_affected_rows(mysql_);
			return std::make_shared<QueryResult>(result, num_fields, num_row);
		}
	}

	LOG_ERROR("query error: %s", sql);

	return nullptr;
}

bool DbMysql::Execute(const char* sql)
{
	if (!inited_)
	{
		if (!Initialize(db_info_.host, db_info_.user, db_info_.passwd, db_info_.db_name))
		{
			return false;
		}
	}

	int query_result = mysql_query(mysql_, sql);

	if (query_result == 0)
	{
		return true;
	}
	else
	{
		unsigned int m_errno = mysql_errno(mysql_);
		if (m_errno == CR_SERVER_GONE_ERROR)
		{
			if (Initialize(db_info_.host, db_info_.user, db_info_.passwd, db_info_.db_name))
			{
				query_result = mysql_query(mysql_, sql);
				if (query_result == 0)
				{
					return true;
				}
			}
		}
	}

	return false;
}
