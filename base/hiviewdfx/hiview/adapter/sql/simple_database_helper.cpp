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
#include "simple_database_helper.h"

namespace OHOS {
namespace HiviewDFX {
bool SimpleDatabaseHelper::CreateTable(sql::SqlTable& sqlTable)
{
    return sqlTable.Create();
}

bool SimpleDatabaseHelper::RemoveTable(sql::SqlTable& sqlTable)
{
    return sqlTable.Remove();
}

bool SimpleDatabaseHelper::TruncateTable(sql::SqlTable& sqlTable)
{
    return sqlTable.Truncate();
}

bool SimpleDatabaseHelper::CreateInfoOnTable(sql::SqlTable& sqlTable, const std::string& infoSql)
{
    return sqlTable.Query(infoSql);
}

size_t SimpleDatabaseHelper::GetRecordCountByFullSql(sql::SqlTable& sqlTable, const std::string& infoSql)
{
    if (infoSql.empty()) {
        return -1;
    }
    sqlTable.Query(infoSql);
    return sqlTable.GetRecordCount();
}

sql::SqlValue* SimpleDatabaseHelper::GetFieldValueByIndex(
    sql::SqlTable& sqlTable, size_t index, size_t recordCount, const std::string& fieldName)
{
    if (index >= recordCount) {
        return nullptr;
    }
    sql::SqlRecord* record = sqlTable.GetRecord(index);
    if (record != nullptr) {
        sql::SqlValue* value = record->GetValue(fieldName);
        return value;
    }
    return nullptr;
}

sql::SqlValue* SimpleDatabaseHelper::GetFieldValueByRecord(sql::SqlRecord& sqlRecord, const std::string& fieldName)
{
    sql::SqlValue* value = sqlRecord.GetValue(fieldName);
    return value;
}

sql::SqlRecord* SimpleDatabaseHelper::GetRecordByKeyId(sql::SqlTable& sqlTable, int keyId)
{
    sql::SqlRecord* record = sqlTable.GetRecordByKeyId(keyId);
    return record;
}

size_t SimpleDatabaseHelper::GetRecordCount(sql::SqlTable& sqlTable, const std::string& querySql)
{
    if (querySql.empty()) {
        sqlTable.Open();
    } else {
        sqlTable.Open(querySql);
    }
    return sqlTable.GetRecordCount();
}

bool SimpleDatabaseHelper::AddRecord(sql::SqlTable& sqlTable, sql::SqlRecord& sqlRecord)
{
    return sqlTable.AddRecord(sqlRecord);
}

bool SimpleDatabaseHelper::UpdateRecord(sql::SqlTable& sqlTable, sql::SqlRecord& sqlRecord)
{
    return sqlTable.UpdateRecord(sqlRecord);
}

bool SimpleDatabaseHelper::DeleteRecord(sql::SqlTable& sqlTable, const std::string& deleteSql)
{
    return sqlTable.DeleteRecords(deleteSql);
}
}  // namespace HiviewDFX
}  // namespace OHOS
