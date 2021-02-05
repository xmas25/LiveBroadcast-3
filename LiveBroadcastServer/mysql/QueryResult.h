//
// Created by rjd67 on 2020/12/27.
//

#ifndef LIVEBROADCASTSERVER_QUERYRESULT_H
#define LIVEBROADCASTSERVER_QUERYRESULT_H

#include <mysql/mysql.h>
#include <map>
#include <memory>
#include "mysql/Field.h"

/**
 * Mysql数据行对象
 *
 * 一个对象代表一行 通过NextRow切换到下一行
 *
 * 一行中有多个列数据Field 每次切换行时自动更新 所有列Field的数据
 */
class QueryResult
{
public:
	QueryResult(MYSQL_RES* res, uint32_t num_fields, uint64_t num_rows);
	~QueryResult();

	/** 获取当前行的某一列数据*/
	const Field& operator[](const std::string& field_name);

	/** 切换到下一行*/
	bool NextRow();

	void EndQuery();
private:
	std::map<std::string, int> field_map_;

	/** 存储当前行的 列数据*/
	Field* fields_;

	MYSQL_RES* res_;

	/**
	 * 列数
	 */
	uint32_t num_fields_;

	/**
	 * 行数
	 */
	uint64_t num_rows_;
};

typedef std::shared_ptr<QueryResult> QueryResultPtr;

#endif //LIVEBROADCASTSERVER_QUERYRESULT_H
