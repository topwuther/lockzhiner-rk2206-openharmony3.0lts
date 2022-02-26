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

#include "sqlite_single_ver_schema_database_upgrader.h"
#include "db_errno.h"
#include "log_print.h"
#include "schema_utils.h"
#include "db_constant.h"

namespace DistributedDB {
SQLiteSingleVerSchemaDatabaseUpgrader::SQLiteSingleVerSchemaDatabaseUpgrader(sqlite3 *db,
    const SchemaObject &newSchema, const SecurityOption &securityOpt, bool isMemDB)
    : SQLiteSingleVerDatabaseUpgrader(db, securityOpt, isMemDB), SingleVerSchemaDatabaseUpgrader(newSchema)
{
}

int SQLiteSingleVerSchemaDatabaseUpgrader::GetDatabaseSchema(std::string &dbSchema) const
{
    int errCode = SQLiteUtils::GetSchema(db_, dbSchema);
    if (errCode != E_OK && errCode != -E_NOT_FOUND) {
        LOGE("[SqlSingleSchemaUp][GetSchema] ErrCode=%d", errCode);
        return errCode;
    }
    return E_OK;
}

int SQLiteSingleVerSchemaDatabaseUpgrader::SetDatabaseSchema(const std::string &dbSchema)
{
    int errCode = SQLiteUtils::SaveSchema(db_, dbSchema);
    if (errCode != E_OK) {
        LOGE("[SqlSingleSchemaUp][SetSchema] ErrCode=%d", errCode);
    }
    return errCode;
}

struct ValueUpgradeContext {
    SchemaObject schema;
    uint32_t checkCount = 0;
    uint32_t getCount = 0;
    int errCode = E_OK;
};

namespace {
const std::string FUNC_NAME_CHECK_AMEND_VALUE = "check_amend_value";
const std::string FUNC_NAME_GET_AMENDED_VALUE = "get_amended_value";
// Current implementation is not of ideal performance: at first, we hope to use check_amend_value to filter out values
// that do not need amend, and call get_amended_value immediately for those value that need amend and obtain amended
// value from ValueUpgradeContext which is cache by check_amend_value just before. It works well for case upgrading from
// kv to schema database, but in the case the original schema having index, the sqlite will gather all rowid of values
// that after filtering at first, then call get_amended_value for each value of rowid later.
// Finally we can only parse value the twice in get_amended_value.
const std::string VALUE_UPGRADE_SQL = "UPDATE sync_data SET value=get_amended_value(value) "
    "WHERE (flag&0x01=0) AND check_amend_value(value) != 0;";
constexpr int USING_STR_LEN = -1;

void CheckGetForJsonSchema(sqlite3_context *ctx, ValueUpgradeContext &context, const RawValue &inValue,
    bool checkTrueGetFalse)
{
    ValueObject valueObj;
    int errCode = valueObj.Parse(inValue.first, inValue.first + inValue.second, context.schema.GetSkipSize());
    if (errCode != E_OK) { // Unlikely
        sqlite3_result_error(ctx, "[SqlSingleSchemaUp][CheckGet] Json value parse fail.", USING_STR_LEN);
        LOGE("[SqlSingleSchemaUp][CheckGet] IsCheck=%d, Json value(cnt=%u) parse fail=%d.", checkTrueGetFalse,
            (checkTrueGetFalse ? context.checkCount : context.getCount), errCode);
        return;
    }
    errCode = context.schema.CheckValueAndAmendIfNeed(ValueSource::FROM_DBFILE, valueObj);
    if (checkTrueGetFalse) {
        if (errCode == -E_VALUE_MATCH) {
            sqlite3_result_int(ctx, 0); // SQLiteResult 0 for check_ok_no_amend
        } else if (errCode == -E_VALUE_MATCH_AMENDED) {
            sqlite3_result_int(ctx, E_VALUE_MATCH_AMENDED); // SQLiteResult not 0 for check_ok_and_amend
        } else {
            sqlite3_result_error(ctx, "[SqlSingleSchemaUp][CheckGet] Json value check fail.", USING_STR_LEN);
            LOGE("[SqlSingleSchemaUp][CheckGet] Json value(cnt=%u) check fail=%d.", context.checkCount, errCode);
            context.errCode = -E_SCHEMA_VIOLATE_VALUE;
        }
    } else {
        if (errCode != -E_VALUE_MATCH_AMENDED) { // Unlikely
            sqlite3_result_error(ctx, "[SqlSingleSchemaUp][CheckGet] Json value no need amend.", USING_STR_LEN);
            LOGE("[SqlSingleSchemaUp][CheckGet] Json value(cnt=%u) no need amend=%d.", context.getCount, errCode);
            context.errCode = -E_INTERNAL_ERROR;
        }
        std::vector<uint8_t> valueAmended;
        valueObj.WriteIntoVector(valueAmended);
        if (valueAmended.size() > DBConstant::MAX_VALUE_SIZE) {
            sqlite3_result_error(ctx, "[SqlSingleSchemaUp][CheckGet] ValSize exceed limit after amend.", USING_STR_LEN);
            LOGE("[SqlSingleSchemaUp][CheckGet] Value(cnt=%u) size=%zu exceed limit after amend.", context.getCount,
                valueAmended.size());
            context.errCode = -E_SCHEMA_VIOLATE_VALUE;
            return;
        }
        // For SQLITE_TRANSIENT, SQLite makes a copy of result into space obtained from sqlite3_malloc before it returns
        sqlite3_result_blob(ctx, valueAmended.data(), valueAmended.size(), SQLITE_TRANSIENT);
    }
}

void CheckGetForFlatBufferSchema(sqlite3_context *ctx, ValueUpgradeContext &context, const RawValue &inValue,
    bool checkTrueGetFalse)
{
    if (!checkTrueGetFalse) {
        sqlite3_result_error(ctx, "[SqlSingleSchemaUp][CheckGet] FlatBuffer value no need amend.", USING_STR_LEN);
        LOGE("[SqlSingleSchemaUp][CheckGet] FlatBuffer value(cnt=%u) no need amend.", context.getCount);
        context.errCode = -E_INTERNAL_ERROR;
    }
    int errCode = context.schema.VerifyValue(ValueSource::FROM_DBFILE, inValue);
    if (errCode != E_OK) {
        sqlite3_result_error(ctx, "[SqlSingleSchemaUp][CheckGet] FlatBuffer value verify fail.", USING_STR_LEN);
        LOGE("[SqlSingleSchemaUp][CheckGet] FlatBuffer value(cnt=%u) verify fail=%d.", context.checkCount, errCode);
        context.errCode = -E_SCHEMA_VIOLATE_VALUE;
        return;
    }
    sqlite3_result_int(ctx, 0); // SQLiteResult 0 for check_ok_no_amend
}

// SQLiteResult 0 for check_ok_no_amend, SQLiteResult not 0 for check_ok_and_amend, SQLiteResult error for check_fail
void CheckValueOrGetAmendValue(sqlite3_context *ctx, int argc, sqlite3_value **argv, bool checkTrueGetFalse)
{
    if (ctx == nullptr || argc != 1 || argv == nullptr) { // 1 parameters, which are value. Unlikely
        LOGE("[SqlSingleSchemaUp][CheckGet] Invalid parameter, argc=%d.", argc);
        return;
    }
    auto context = static_cast<ValueUpgradeContext *>(sqlite3_user_data(ctx));
    if (context == nullptr || !context->schema.IsSchemaValid()) { // Unlikely
        sqlite3_result_error(ctx, "[SqlSingleSchemaUp][CheckGet] No context or schema invalid.", USING_STR_LEN);
        LOGE("[SqlSingleSchemaUp][CheckGet] No context or schema invalid.");
        return;
    }
    auto valueBlob = static_cast<const uint8_t *>(sqlite3_value_blob(argv[0]));
    int valueBlobLen = sqlite3_value_bytes(argv[0]);
    if ((valueBlob == nullptr) || (valueBlobLen <= 0)) { // Is delete record, Unlikely
        // Currently delete records are filtered out of value upgrade sql, so not allowed here.
        sqlite3_result_error(ctx, "[SqlSingleSchemaUp][CheckGet] Delete record not allowed.", USING_STR_LEN);
        LOGE("[SqlSingleSchemaUp][CheckGet] Delete record not allowed.");
        return;
    }

    if (context->schema.GetSchemaType() == SchemaType::JSON) {
        CheckGetForJsonSchema(ctx, *context, RawValue{valueBlob, valueBlobLen}, checkTrueGetFalse);
    } else {
        CheckGetForFlatBufferSchema(ctx, *context, RawValue{valueBlob, valueBlobLen}, checkTrueGetFalse);
    }
    // Count only for non-delete value in check_func or get_func
    if (checkTrueGetFalse) {
        context->checkCount++;
    } else {
        context->getCount++;
    }
}

void CheckAmendValue(sqlite3_context *ctx, int argc, sqlite3_value **argv)
{
    CheckValueOrGetAmendValue(ctx, argc, argv, true);
}

void GetAmendedValue(sqlite3_context *ctx, int argc, sqlite3_value **argv)
{
    CheckValueOrGetAmendValue(ctx, argc, argv, false);
}
}

int SQLiteSingleVerSchemaDatabaseUpgrader::UpgradeValues()
{
    ValueUpgradeContext context;
    context.schema = newSchema_;
    LOGD("[SqlSingleSchemaUp][UpValue] Begin.");
    int errCode = sqlite3_create_function_v2(db_, FUNC_NAME_CHECK_AMEND_VALUE.c_str(),
        1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, &context, &CheckAmendValue, nullptr, nullptr, nullptr); // 1 args
    if (errCode != SQLITE_OK) {
        LOGE("[SqlSingleSchemaUp][UpValue] Create func=check_amend_value return=%d.", errCode);
        return SQLiteUtils::MapSQLiteErrno(errCode);
    }
    // GetAmendedValue is better not be of deterministic type, otherwise sqlite may take it as constant
    errCode = sqlite3_create_function_v2(db_, FUNC_NAME_GET_AMENDED_VALUE.c_str(),
        1, SQLITE_UTF8, &context, &GetAmendedValue, nullptr, nullptr, nullptr); // 1 args
    if (errCode != SQLITE_OK) {
        LOGE("[SqlSingleSchemaUp][UpValue] Create func=get_amended_value return=%d.", errCode);
        return SQLiteUtils::MapSQLiteErrno(errCode);
    }
    errCode = SQLiteUtils::ExecuteRawSQL(db_, VALUE_UPGRADE_SQL);
    if (errCode != E_OK) {
        LOGE("[SqlSingleSchemaUp][UpValue] Execute value upgrade fail=%d, contextErr=%d.", errCode, context.errCode);
        // If error caused by upgrade nor sqlite, using contextErr as the final errCode
        errCode = ((context.errCode == E_OK ? errCode : context.errCode));
    }
    LOGD("[SqlSingleSchemaUp][UpValue] End.");
    return errCode;
}

int SQLiteSingleVerSchemaDatabaseUpgrader::UpgradeIndexes(const IndexDifference &indexDiffer)
{
    uint32_t skipSize = newSchema_.GetSkipSize();
    SchemaType theType = newSchema_.GetSchemaType();
    // The order of index upgrade is not compulsory, we think order "decrease, change, increase" may be better.
    for (const auto &entry : indexDiffer.decrease) {
        LOGI("[SqlSingleSchemaUp][UpIndex] DecreaseIndex : indexName=%s.", SchemaUtils::FieldPathString(entry).c_str());
        int errCode = SQLiteUtils::DecreaseIndex(db_, entry);
        if (errCode != E_OK) {
            LOGE("[SqlSingleSchemaUp][UpIndex] DecreaseIndex fail, errCode=%d.", errCode);
            return errCode;
        }
    }
    for (const auto &entry : indexDiffer.change) {
        LOGI("[SqlSingleSchemaUp][UpIndex] ChangeIndex : SkipSize=%u, indexName=%s, fieldCount=%zu, type=%s.",
            skipSize, SchemaUtils::FieldPathString(entry.first).c_str(), entry.second.size(),
            SchemaUtils::SchemaTypeString(theType).c_str());
        int errCode = SQLiteUtils::ChangeIndex(db_, entry.first, entry.second, theType, skipSize);
        if (errCode != E_OK) {
            LOGE("[SqlSingleSchemaUp][UpIndex] ChangeIndex fail, errCode=%d.", errCode);
            return errCode;
        }
    }
    for (const auto &entry : indexDiffer.increase) {
        LOGI("[SqlSingleSchemaUp][UpIndex] IncreaseIndex : SkipSize=%u, indexName=%s, fieldCount=%zu, type=%s.",
            skipSize, SchemaUtils::FieldPathString(entry.first).c_str(), entry.second.size(),
            SchemaUtils::SchemaTypeString(theType).c_str());
        int errCode = SQLiteUtils::IncreaseIndex(db_, entry.first, entry.second, theType, skipSize);
        if (errCode != E_OK) {
            LOGE("[SqlSingleSchemaUp][UpIndex] IncreaseIndex fail, errCode=%d.", errCode);
            return errCode;
        }
    }
    return E_OK;
}
} // namespace DistributedDB
