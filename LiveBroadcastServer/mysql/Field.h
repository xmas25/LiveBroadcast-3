//
// Created by rjd67 on 2020/12/27.
//

#ifndef LIVEBROADCASTSERVER_FIELD_H
#define LIVEBROADCASTSERVER_FIELD_H

#include <string>

/**
 * 列对象 存储某行的一个列数据
 *
 * 列数据类型初次设定后 一般不需要改变
 *
 * 数据data_ 由QueryResult的NextRow更新
 */
class Field
{
public:
	enum DataType
	{
		DATA_TYPE_UNKNOWN,
		DATA_TYPE_INTEGER,
		DATA_TYPE_STRING,
		DATA_TYPE_BOOL,
		DATA_TYPE_FLOAT
	};

	Field();
	~Field();

	void SetDataType(DataType data_type);

	void SetData(const char* data, size_t length);

	void SetNullptr(bool nullptr_data);

	std::string GetString() const;

	const char* GetCStr() const;
private:

	DataType data_type_;

	std::string data_;
	bool nullptr_data_;
};


#endif //LIVEBROADCASTSERVER_FIELD_H
