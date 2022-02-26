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
#include "sql_operator.h"

#include "defines.h"
#include "logger.h"
#include "sql_wrapper.h"

using namespace std;

namespace sql {
DEFINE_LOG_TAG("HiView-SqlOperator");

SqlOperator::SqlOperator(const Sqlite3Db& indb): db(indb)
{
    errMsg.clear();
    execResult = SQLITE_ERROR;
    fieldsVector = nullptr;
    fieldsMap = nullptr;
    records.clear();
}

SqlOperator::SqlOperator(const Sqlite3Db& indb, std::vector<SqlField*>& fieldsVector,
    std::map<string, SqlField*>& fieldsMap)
    : db(indb), fieldsVector(&fieldsVector), fieldsMap(&fieldsMap)
{
    errMsg.clear();
    execResult = SQLITE_ERROR;
    records.clear();
}

size_t SqlOperator::GetCount()
{
    return records.size();
}

SqlOperator::~SqlOperator(void)
{
    Clear();
}

void SqlOperator::Clear()
{
    errMsg.clear();
    execResult = SQLITE_ERROR;
    records.clear();
}

bool SqlOperator::IsOk()
{
    return (execResult == SQLITE_OK);
}

int SqlOperator::LoadRecord(void* param, int columnCount, char** values, char** columnNames __UNUSED)
{
    SqlOperator* oprt = reinterpret_cast<SqlOperator*>(param);
    if ((oprt != nullptr) && (oprt->fieldsVector != nullptr) && values != nullptr) {
        size_t count = oprt->fieldsVector->size();
        SqlRecord record(*(oprt->fieldsMap));
        record.InitColumnCount(columnCount);

        for (size_t index = 0; (index < static_cast<size_t>(columnCount)) && (index < count); index++) {
            string value = "";
            if (values[index] != nullptr) {
                value = values[index];
            }
            if (SqlField* field = oprt->fieldsVector->at(index)) {
                record.InitColumnValue(index, value, field->GetType());
            }
        }

        oprt->records.push_back(record);
    }

    return DATASET_ITERATION_CONTINUE;
}

bool SqlOperator::Execute(string sql)
{
    Clear();

    if (db.rawdb == nullptr) {
        HIVIEW_LOGE("Execute db is null");
        return false;
    }

    char* error = nullptr;
    execResult = sqlite3_exec(const_cast<sqlite3*>(db.rawdb), sql.c_str(), LoadRecord, this, &error);

    if (IsOk()) {
        return true;
    }

    if (error == nullptr) {
        errMsg = "";
        HIVIEW_LOGE("Execute error returns null");
        return false;
    }

    if (*error) {
        errMsg = error;
        sqlite3_free(error);
        HIVIEW_LOGE("Execute sql failed: %s", errMsg.c_str());
    }

    return false;
}

bool SqlOperator::ExecuteNoCallback(string sql)
{
    Clear();

    if (db.rawdb == nullptr) {
        HIVIEW_LOGE("ExecuteNoCallback db is null");
        return false;
    }

    char* error = nullptr;
    execResult = sqlite3_exec(const_cast<sqlite3*>(db.rawdb), sql.c_str(), NULL, NULL, &error);

    if (IsOk()) {
        return true;
    }

    if (error == nullptr) {
        errMsg = "";
        HIVIEW_LOGE("ExecuteNoCallback error returns null");
        return false;
    }

    if (*error) {
        errMsg = error;
        sqlite3_free(error);
        HIVIEW_LOGE("ExecuteNoCallback sql failed: %s", errMsg.c_str());
    }

    return false;
}

SqlRecord* SqlOperator::GetRecord(size_t index)
{
    if (index < records.size()) {
        return &records.at(index);
    }
    return nullptr;
}

SqlRecord* SqlOperator::GetTopRecord()
{
    if (IsOk()) {
        return GetRecord(0);
    }
    return nullptr;
}

SqlValue* SqlOperator::GetTopRecordFirstValue()
{
    if (IsOk()) {
        if (SqlRecord* record = GetRecord(0)) {
            return record->GetValue(0);
        }
    }
    return nullptr;
}
};  // namespace sql
