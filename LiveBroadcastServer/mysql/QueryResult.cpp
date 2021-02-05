//
// Created by rjd67 on 2020/12/27.
//

#include "QueryResult.h"

inline Field::DataType ConvertNativeType(enum_field_types type)
{
	switch (type)
	{
		case FIELD_TYPE_TIMESTAMP:
		case FIELD_TYPE_DATE:
		case FIELD_TYPE_TIME:
		case FIELD_TYPE_DATETIME:
		case FIELD_TYPE_YEAR:
		case FIELD_TYPE_STRING:
		case FIELD_TYPE_VAR_STRING:
		case FIELD_TYPE_BLOB:
		case FIELD_TYPE_SET:
		case FIELD_TYPE_NULL:
			return Field::DATA_TYPE_STRING;
		case FIELD_TYPE_TINY:
		case FIELD_TYPE_SHORT:
		case FIELD_TYPE_LONG:
		case FIELD_TYPE_INT24:
		case FIELD_TYPE_LONGLONG:
		case FIELD_TYPE_ENUM:
			return Field::DATA_TYPE_INTEGER;
		case FIELD_TYPE_DECIMAL:
		case FIELD_TYPE_FLOAT:
		case FIELD_TYPE_DOUBLE:
			return Field::DATA_TYPE_FLOAT;
		default:
			return Field::DATA_TYPE_UNKNOWN;
	}

}

QueryResult::QueryResult(MYSQL_RES* res, uint32_t num_fields, uint64_t num_rows):
	fields_(new Field[num_fields]),
	res_(res),
	num_fields_(num_fields),
	num_rows_(num_rows)
{
	if (res)
	{
		MYSQL_FIELD* fields = mysql_fetch_field(res);
		for (uint32_t i = 0; i < num_fields_; ++i)
		{
			const MYSQL_FIELD& field = fields[i];
			field_map_.emplace(field.name, i);

			fields_->SetDataType(ConvertNativeType(field.type));
		}

		NextRow();
	}
}

QueryResult::~QueryResult()
{
	EndQuery();
}

const Field& QueryResult::operator[](const std::string& field_name)
{
	return fields_[field_map_[field_name]];
}

bool QueryResult::NextRow()
{
	if (!res_)
	{
		return false;
	}
	MYSQL_ROW row = mysql_fetch_row(res_);

	// 遍历结束
	if (!row)
	{
		EndQuery();
		return false;
	}

	/** 当前行 每列数据的数据长度*/
	unsigned long int* field_length_ = mysql_fetch_lengths(res_);

	/** 更新当前行的所有列数据*/
	for (uint32_t i = 0; i < num_fields_; ++i)
	{
		const char* data = row[i];
		if (data)
		{
			fields_[i].SetNullptr(false);
			fields_[i].SetData(data, field_length_[i]);
		}
		else
		{
			fields_[i].SetNullptr(true);
			fields_[i].SetData("", 0);
		}
	}
	return true;
}

void QueryResult::EndQuery()
{
	if (fields_)
	{
		delete[] fields_;
		fields_ = nullptr;
	}
	if (res_)
	{
		mysql_free_result(res_);
		res_ = nullptr;
	}
}