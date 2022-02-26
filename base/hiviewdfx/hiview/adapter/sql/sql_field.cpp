/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "sql_field.h"

using namespace std;

namespace sql {
SqlField::SqlField(string name, FieldType type, FieldFlag isPrimaryKey, FieldFlag isNull)
{
    this->name = name;
    this->type = type;
    this->isPrimaryKey = (isPrimaryKey == FIELD_PRIMARY_KEY);
    this->isNull = (isNull != FIELD_NOT_NULL);
    this->index = INVALID_INDEX;
    this->isEndingField = false;
}

SqlField::SqlField(string name, FieldType type)
{
    this->name = name;
    this->type = type;
    this->isPrimaryKey = false;
    this->isNull = true;
    this->index = INVALID_INDEX;
    this->isEndingField = false;
}

SqlField::SqlField(const SqlField& value)
{
    this->name = value.name;
    this->type = value.type;
    this->isPrimaryKey = value.isPrimaryKey;
    this->isNull = value.isNull;
    this->index = value.index;
    this->isEndingField = value.isEndingField;
}

SqlField::SqlField(FieldUse isEndingField)
{
    this->name = "";
    this->type = TYPE_UNDEFINED;
    this->isPrimaryKey = false;
    this->isNull = true;
    this->index = INVALID_INDEX;
    this->isEndingField = (isEndingField == FIELD_END) ? true : false;
}

string SqlField::GetName() const
{
    return name;
}

size_t SqlField::GetIndex()
{
    return index;
}

void SqlField::setIndex(size_t index)
{
    this->index = index;
}

FieldType SqlField::GetType()
{
    return type;
}

string SqlField::GetTypeStr()
{
    switch (type) {
        case TYPE_INT:
            return "INTEGER";
        case TYPE_TEXT:
            return "TEXT";
        case TYPE_FLOAT:
            return "REAL";
        default:
            return "";
    }
}

bool SqlField::IsPrimaryKey()
{
    return isPrimaryKey;
}

bool SqlField::IsNull()
{
    return isNull;
}

string SqlField::GetDefinition()
{
    string value = name + " " + GetTypeStr();
    if (IsPrimaryKey()) {
        value += " PRIMARY KEY";
    }
    if (!IsNull()) {
        value += " NOT NULL";
    }
    return TrimLeft(value);
}

bool SqlField::IsEndingField()
{
    return isEndingField;
}
};  // namespace sql
