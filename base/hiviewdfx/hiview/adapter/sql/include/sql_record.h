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
#ifndef SQL_RECORD_H
#define SQL_RECORD_H

#include "sql_value.h"
#include "sql_field.h"

namespace sql {
using std::string;
class SqlRecord {
public:
    SqlRecord(const std::map<string, SqlField*>& fields);
    SqlRecord(const SqlRecord& record);
    void InitColumnCount(size_t columns);
    void InitColumnValue(size_t index, string& value, FieldType type);
    virtual ~SqlRecord()
    {
        fields = nullptr;
    }

public:
    size_t GetColumnCount();
    SqlValue* GetValue(size_t index);
    SqlValue* GetValue(string fieldName);
    SqlValue* GetValue(const SqlField& field);
    SqlField* GetFieldByName(string fieldName);
    string ToSql(std::vector<SqlField*>& sqlFieldVector);

public:
    void SetNull(size_t index);
    void SetString(size_t index, string value);
    void SetInteger(size_t index, Integer value);
    void SetDouble(size_t index, double value);

public:
    void SetNull(string fieldName);
    void SetString(string fieldName, string value);
    void SetString(string fieldName, string value, int length);
    void SetInteger(string fieldName, Integer value);
    void SetDouble(string fieldName, double value);

public:
    void SetNull(SqlField& field);
    void SetString(SqlField& field, string value);
    void SetInteger(SqlField& field, Integer value);
    void SetDouble(SqlField& field, double value);

private:
    const std::map<string, SqlField*>* fields;
    std::vector<SqlValue> values;
};
};  // namespace sql

#endif
