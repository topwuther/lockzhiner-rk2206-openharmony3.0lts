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
#ifndef SQL_TABLE_H
#define SQL_TABLE_H

#include "sql_operator.h"

namespace sql {
using std::string;
struct Sqlite3Db;
class SqlTable {
public:
    SqlTable(const Sqlite3Db& db, string tableName, SqlField definition[], size_t length);
    virtual ~SqlTable(){};

public:
    string GetName();
    string GetDefinition();
    string GetFieldsDefinition();
    void InitFields(SqlField definition[], size_t length);
    const std::map<string, SqlField*>& GetFields();
    string GetFieldsString();

public:
    bool Create();
    bool Exists();
    bool Remove();

public:
    bool Open();
    bool Open(string whereCondition);
    int GetTotalRecordCount();

public:
    size_t GetRecordCount();
    SqlRecord* GetRecord(size_t recordIndex);
    SqlRecord* GetTopRecord();
    SqlRecord* GetRecordByKeyId(Integer keyId);

public:
    bool Query(string sql);
    string GetAddString(SqlRecord& record);
    bool AddRecord(SqlRecord& record);
    string GetUpdateString(SqlRecord& record);
    bool UpdateRecord(SqlRecord& record);
    bool DeleteRecords(string whereCondition);
    bool Truncate();

private:
    string tableName;
    SqlOperator sqlOperator;
    std::vector<SqlField*> sqlFieldVector;
    std::map<string, SqlField*> sqlFieldMap;
};
};  // namespace sql

#endif
