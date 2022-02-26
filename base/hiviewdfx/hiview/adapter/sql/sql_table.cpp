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
#include "sql_table.h"

#include "logger.h"
#include "sql_wrapper.h"

using namespace std;

namespace sql {
SqlTable::SqlTable(const Sqlite3Db& db, string tableName, SqlField definition[], size_t length)
    : tableName(tableName), sqlOperator(db, sqlFieldVector, sqlFieldMap)
{
    InitFields(definition, length);
}

string SqlTable::GetName()
{
    return tableName;
}

string SqlTable::GetDefinition()
{
    return "CREATE TABLE " + tableName + " (" + GetFieldsDefinition() + ")";
}

void SqlTable::InitFields(SqlField definition[], size_t length)
{
    if (definition) {
        size_t index = 0;
        while (index < length) {
            SqlField& field = definition[index];
            if (field.IsEndingField()) {
                break;
            }
            field.setIndex(index);
            sqlFieldVector.push_back(&field);
            sqlFieldMap[field.GetName()] = &field;
            index++;
        }
    }
}

const std::map<string, SqlField*>& SqlTable::GetFields()
{
    return sqlFieldMap;
}

string SqlTable::GetFieldsDefinition()
{
    string fieldsDef = "";
    size_t count = sqlFieldVector.size();
    for (size_t index = 0; index < count; index++) {
        SqlField* field = sqlFieldVector[index];
        fieldsDef += field->GetDefinition();
        if (index < (count - 1)) {
            fieldsDef += ", ";
        }
    }
    return fieldsDef;
}

string SqlTable::GetFieldsString()
{
    string fieldsStr = "";
    size_t count = sqlFieldVector.size();
    for (size_t index = 0; index < count; index++) {
        SqlField* field = sqlFieldVector[index];
        fieldsStr += field->GetName();
        if (index < (count - 1)) {
            fieldsStr += ", ";
        }
    }
    return fieldsStr;
}

bool SqlTable::Create()
{
    if (Exists()) {
        return true;
    }
    const string sql = GetDefinition();
    return sqlOperator.ExecuteNoCallback(sql);
}

bool SqlTable::Remove()
{
    if (!Exists()) {
        return true;
    }
    const string sql = "drop table " + tableName;
    if (sqlOperator.ExecuteNoCallback(sql)) {
        return true;
    }
    return false;
}

bool SqlTable::Exists()
{
    const string sql = "select count(*) from sqlite_master where type = 'table' and name = '" + tableName + "'";
    if (sqlOperator.Execute(sql)) {
        if (SqlValue* value = sqlOperator.GetTopRecordFirstValue()) {
            return (value->AsInteger() > 0);
        }
    }
    return false;
}

bool SqlTable::Open()
{
    if (!Exists()) {
        return false;
    }
    const string sql = "select * from " + tableName;
    if (sqlOperator.Execute(sql)) {
        return true;
    }
    return false;
}

bool SqlTable::Open(string whereCondition)
{
    if (!Exists()) {
        return false;
    }
    const string sql = "select * from " + tableName + (whereCondition.empty() ? "" : (" where " + whereCondition));
    return sqlOperator.Execute(sql);
}

bool SqlTable::Query(string sql)
{
    return sqlOperator.Execute(sql);
}

bool SqlTable::DeleteRecords(string whereCondition)
{
    if (!Exists()) {
        return false;
    }
    const string sql = "delete from " + tableName + (whereCondition.empty() ? "" : (" where " + whereCondition));
    return sqlOperator.ExecuteNoCallback(sql);
}

bool SqlTable::Truncate()
{
    if (!Exists()) {
        return false;
    }
    const string sql = "delete from " + tableName;
    return sqlOperator.ExecuteNoCallback(sql);
}

size_t SqlTable::GetRecordCount()
{
    return sqlOperator.GetCount();
}

int SqlTable::GetTotalRecordCount()
{
    const string sql = "select count(*) from " + tableName;
    if (sqlOperator.Execute(sql)) {
        if (SqlValue* value = sqlOperator.GetTopRecordFirstValue()) {
            return static_cast<int>(value->AsInteger());
        }
    }
    return -1;
}

SqlRecord* SqlTable::GetRecord(size_t index)
{
    return sqlOperator.GetRecord(index);
}

SqlRecord* SqlTable::GetTopRecord()
{
    return GetRecord(0);
}

SqlRecord* SqlTable::GetRecordByKeyId(Integer keyId)
{
    if (!Exists()) {
        return nullptr;
    }
    const string sql = "select * from " + tableName + " where _ID = " + IntegerToStr(keyId);
    if (sqlOperator.Execute(sql)) {
        if (sqlOperator.GetCount() > 0) {
            return sqlOperator.GetRecord(0);
        }
    }
    return nullptr;
}

string SqlTable::GetAddString(SqlRecord& record)
{
    string str = "insert into " + tableName + " ";
    str += "(" + GetFieldsString() + ")";
    str += " values ";
    str += "(" + record.ToSql(sqlFieldVector) + ")";
    return str;
}

bool SqlTable::AddRecord(SqlRecord& record)
{
    const string sql = GetAddString(record);
    return sqlOperator.ExecuteNoCallback(sql);
}

string SqlTable::GetUpdateString(SqlRecord& record)
{
    string str = "update " + tableName + " set ";
    size_t count = sqlFieldVector.size();
    SqlField* field = nullptr;
    SqlValue* value = nullptr;
    // ignore the _ID field
    for (size_t index = 1; index < count; index++) {
        field = sqlFieldVector[index];
        if (field == nullptr) {
            continue;
        }
        value = record.GetValue(field->GetName());
        if (value == nullptr) {
            continue;
        }
        str += field->GetName() + "=" + value->ToSql(field->GetType());
        if (index < (count - 1)) {
            str += ", ";
        }
    }
    // get _ID field
    field = sqlFieldVector[0];
    if (field != nullptr) {
        value = record.GetValue(field->GetName());
        if (value != nullptr) {
            str += " where _ID = " + value->ToSql(field->GetType());
        }
    }
    return str;
}

bool SqlTable::UpdateRecord(SqlRecord& record)
{
    const string sql = GetUpdateString(record);
    return sqlOperator.ExecuteNoCallback(sql);
}
};  // namespace sql
