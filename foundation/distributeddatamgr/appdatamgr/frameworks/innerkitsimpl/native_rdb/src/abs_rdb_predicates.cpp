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

#include "abs_rdb_predicates.h"

#include "logger.h"

namespace OHOS {
namespace NativeRdb {
AbsRdbPredicates::AbsRdbPredicates(std::string tableName)
{
    if (tableName.empty()) {
        this->tableName = "";
        LOG_INFO("no tableName specified.");
        return;
    }
    this->tableName = tableName;
}

/**
 * Obtains the table name.
 */
std::string AbsRdbPredicates::GetTableName() const
{
    return tableName;
}

std::string AbsRdbPredicates::ToString() const
{
    std::string args;
    for (std::string item : GetWhereArgs()) {
        args += item + ", ";
    }
    return "TableName = " + GetTableName() + ", {WhereClause:" + GetWhereClause() + ", whereArgs:{" + args + "}"
           + ", order:" + GetOrder() + ", group:" + GetGroup() + ", index:" + GetIndex()
           + ", limit:" + std::to_string(GetLimit()) + ", offset:" + std::to_string(GetOffset())
           + ", distinct:" + std::to_string(IsDistinct()) + ", isNeedAnd:" + std::to_string(IsNeedAnd())
           + ", isSorted:" + std::to_string(IsSorted()) + "}";
}
} // namespace NativeRdb
} // namespace OHOS