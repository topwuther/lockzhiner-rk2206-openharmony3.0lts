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

#include "statement.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "Statement"};
}

Statement::Statement(sqlite3* db, const std::string& sql) : db_(db), sql_(sql)
{
    if (sqlite3_prepare_v2(db, sql.c_str(), sql.size(), &statement_, nullptr) != SQLITE_OK) {
        PERMISSION_LOG_ERROR(LABEL, "Cannot prepare, errorMsg: %{public}s", sqlite3_errmsg(db_));
    }
}

Statement::~Statement()
{
    sqlite3_finalize(statement_);
    statement_ = nullptr;
}

void Statement::Bind(const int index, const std::string& text)
{
    if (sqlite3_bind_text(statement_, index, text.c_str(), text.size(), SQLITE_TRANSIENT) != SQLITE_OK) {
        PERMISSION_LOG_ERROR(LABEL, "Cannot bind string, errorMsg: %{public}s", sqlite3_errmsg(db_));
    }
}

void Statement::Bind(const int index, int value)
{
    if (sqlite3_bind_int(statement_, index, value) != SQLITE_OK) {
        PERMISSION_LOG_ERROR(LABEL, "Cannot bind int, errorMsg: %{public}s", sqlite3_errmsg(db_));
    }
}

int Statement::GetColumnInt(const int column) const
{
    return sqlite3_column_int(statement_, column);
}

std::string Statement::GetColumnString(const int column) const
{
    return std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement_, column)));
}

std::string Statement::GetColumnName(const int column) const
{
    return sqlite3_column_name(statement_, column);
}

Statement::State Statement::Step()
{
    int ret = sqlite3_step(statement_);
    switch (ret) {
        case SQLITE_ROW:
            return Statement::State::ROW;
        case SQLITE_DONE:
            return Statement::State::DONE;
        case SQLITE_BUSY:
            return Statement::State::BUSY;
        case SQLITE_MISUSE:
            return Statement::State::MISUSE;
        default:
            return Statement::State::UNKNOWN;
    }
}

int Statement::GetParameterIndex(const std::string& name) const
{
    return sqlite3_bind_parameter_index(statement_, name.c_str());
}

void Statement::Bind(const std::string& tableColumnName, const VariantValue& value)
{
    int index = GetParameterIndex(":" + tableColumnName);
    if (value.GetType() == ValueType::TYPE_STRING) {
        Bind(index, value.GetString());
    } else if (value.GetType() == ValueType::TYPE_INT) {
        Bind(index, value.GetInt());
    }
}

int Statement::Reset()
{
    return sqlite3_reset(statement_);
}

int Statement::GetColumnCount() const
{
    return sqlite3_column_count(statement_);
}

VariantValue Statement::GetValue(const int column) const
{
    int type = sqlite3_column_type(statement_, column);
    switch (type) {
        case SQLITE_INTEGER:
            return VariantValue(GetColumnInt(column));
        case SQLITE_TEXT:
            return VariantValue(GetColumnString(column));
        default:
            return VariantValue();
    }
}
} // namespace Permission
} // namespace Security
} // namespace OHOS