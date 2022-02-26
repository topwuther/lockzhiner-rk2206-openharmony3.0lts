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
#include "sql_record.h"

#include "defines.h"

using namespace std;

namespace sql {
SqlRecord::SqlRecord(const std::map<string, SqlField*>& fields) : fields(&fields)
{
    InitColumnCount(fields.size());
}

SqlRecord::SqlRecord(const SqlRecord& record) : fields(record.fields)
{
    if (fields != nullptr) {
        InitColumnCount(fields->size());
        values = record.values;
    }
}

void SqlRecord::InitColumnCount(size_t columns)
{
    values.resize(columns);
}

void SqlRecord::InitColumnValue(size_t index, string& value, FieldType type)
{
    values[index].SetValue(value, type);
}

size_t SqlRecord::GetColumnCount()
{
    return values.size();
}

SqlValue* SqlRecord::GetValue(size_t index)
{
    if (index < values.size()) {
        return &values.at(index);
    }
    return nullptr;
}

SqlValue* SqlRecord::GetValue(string fieldName)
{
    if (fields == nullptr) {
        return nullptr;
    }

    std::map<string, SqlField*>::const_iterator iter = fields->find(fieldName);
    if (iter != fields->end()) {
        SqlField* field = iter->second;
        if (field != nullptr) {
            return GetValue(field->GetIndex());
        }
    }

    return nullptr;
}

SqlValue* SqlRecord::GetValue(const SqlField& field)
{
    return GetValue(field.GetName());
}

string SqlRecord::ToSql(std::vector<SqlField*>& sqlFieldVector)
{
    if (sqlFieldVector.empty()) {
        return "";
    }

    string str;
    size_t count = sqlFieldVector.size();
    SqlField* field = nullptr;
    SqlValue* value = nullptr;
    for (size_t index = 0; index < count; index++) {
        field = sqlFieldVector[index];
        if (field == nullptr) {
            continue;
        }

        value = GetValue(field->GetName());
        if (value == nullptr) {
            continue;
        }

        str += value->ToSql(field->GetType());
        if (index < (count - 1)) {
            str += ", ";
        }
    }
    return str;
}

void SqlRecord::SetNull(size_t index)
{
    if (SqlValue* value = GetValue(index)) {
        value->SetNull();
    }
}

void SqlRecord::SetString(size_t index, string value)
{
    if (SqlValue* sqlValue = GetValue(index)) {
        sqlValue->SetString(value);
    }
}

void SqlRecord::SetInteger(size_t index, Integer value)
{
    if (SqlValue* sqlValue = GetValue(index)) {
        sqlValue->SetInteger(value);
    }
}

void SqlRecord::SetDouble(size_t index, double value)
{
    if (SqlValue* sqlValue = GetValue(index)) {
        sqlValue->SetDouble(value);
    }
}

SqlField* SqlRecord::GetFieldByName(string fieldName)
{
    if (fields == nullptr) {
        return nullptr;
    }
    std::map<string, SqlField*>::const_iterator iter = fields->find(fieldName);
    if (iter != fields->end()) {
        SqlField* field = iter->second;
        return field;
    }
    return nullptr;
}

void SqlRecord::SetNull(string fieldName)
{
    if (SqlField* field = GetFieldByName(fieldName)) {
        SetNull(field->GetIndex());
    }
}

void SqlRecord::SetString(string fieldName, string value)
{
    if (SqlField* field = GetFieldByName(fieldName)) {
        SetString(field->GetIndex(), value);
    }
}

void SqlRecord::SetString(string fieldName, string value, int length __UNUSED)
{
    if (SqlField* field = GetFieldByName(fieldName)) {
        SetString(field->GetIndex(), value);
    }
}

void SqlRecord::SetInteger(string fieldName, Integer value)
{
    if (SqlField* field = GetFieldByName(fieldName)) {
        SetInteger(field->GetIndex(), value);
    }
}

void SqlRecord::SetDouble(string fieldName, double value)
{
    if (SqlField* field = GetFieldByName(fieldName)) {
        SetDouble(field->GetIndex(), value);
    }
}

void SqlRecord::SetNull(SqlField& field)
{
    SetNull(field.GetName());
}

void SqlRecord::SetString(SqlField& field, string value)
{
    SetString(field.GetName(), value);
}

void SqlRecord::SetInteger(SqlField& field, Integer value)
{
    SetInteger(field.GetName(), value);
}

void SqlRecord::SetDouble(SqlField& field, double value)
{
    SetDouble(field.GetName(), value);
}
};  // namespace sql
