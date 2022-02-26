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
#ifndef SQL_OPERATOR_H
#define SQL_OPERATOR_H

#include "sql_record.h"

namespace sql {
using std::string;
struct Sqlite3Db;
class SqlOperator {
public:
    SqlOperator(const Sqlite3Db& db);
    SqlOperator(const Sqlite3Db& db, std::vector<SqlField*>& fieldsVector, std::map<string, SqlField*>& fieldsMap);
    virtual ~SqlOperator(void);

public:
    bool IsOk();
    bool Execute(string sql);
    bool ExecuteNoCallback(string sql);
    void Clear();

public:
    size_t GetCount();
    SqlRecord* GetRecord(size_t index);
    SqlRecord* GetTopRecord();
    SqlValue* GetTopRecordFirstValue();

private:
    enum {
        DATASET_ITERATION_CONTINUE = 0,
        DATASET_ITERATION_ABORT = 1,
    };

private:
    const Sqlite3Db& db;
    string errMsg;
    int execResult;
    std::vector<SqlField*>* fieldsVector;
    std::map<string, SqlField*>* fieldsMap;
    std::vector<SqlRecord> records;

private:
    // this is callback function for sqlite3_exec, so must use void* param.
    static int LoadRecord(void* param, int columnCount, char** values, char** columnNames);
};
};  // namespace sql

#endif
