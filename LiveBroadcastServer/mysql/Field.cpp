//
// Created by rjd67 on 2020/12/27.
//

#include "Field.h"

Field::Field():
	data_type_(DATA_TYPE_UNKNOWN),
	data_(),
	nullptr_data_(true)
{

}

Field::~Field()
{

}

void Field::SetDataType(Field::DataType data_type)
{
	data_type_ = data_type;
}

void Field::SetData(const char* data, size_t length)
{
	data_.assign(data, length);
}

void Field::SetNullptr(bool nullptr_data)
{
	nullptr_data_ = nullptr_data;
}

std::string Field::GetString() const
{
	return data_;
}

const char* Field::GetCStr() const
{
	return data_.c_str();
}
