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
#ifndef SIMPLE_DATABASE_HELPER_H
#define SIMPLE_DATABASE_HELPER_H

#include <string>

#include "sql_helper.h"

namespace OHOS {
namespace HiviewDFX {
class SimpleDatabaseHelper {
public:
    bool CreateTable(sql::SqlTable& sqlTable);
    bool RemoveTable(sql::SqlTable& sqlTable);
    bool TruncateTable(sql::SqlTable& sqlTable);
    bool CreateInfoOnTable(sql::SqlTable& sqlTable, const std::string& infoSql);
    size_t GetRecordCountByFullSql(sql::SqlTable& sqlTable, const std::string& infoSql);
    sql::SqlValue* GetFieldValueByIndex(
        sql::SqlTable& sqlTable, size_t index, size_t recordCount, const std::string& fieldName);
    sql::SqlValue* GetFieldValueByRecord(sql::SqlRecord& sqlRecord, const std::string& fieldName);
    sql::SqlRecord* GetRecordByKeyId(sql::SqlTable& sqlTable, int keyId);
    size_t GetRecordCount(sql::SqlTable& sqlTable, const std::string& querySql);
    bool AddRecord(sql::SqlTable& sqlTable, sql::SqlRecord& sqlRecord);
    bool UpdateRecord(sql::SqlTable& sqlTable, sql::SqlRecord& sqlRecord);
    bool DeleteRecord(sql::SqlTable& sqlTable, const std::string& deleteSql);
};
}  // namespace HiviewDFX
}  // namespace OHOS

#endif
