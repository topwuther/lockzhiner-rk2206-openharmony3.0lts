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

#ifndef SQLITE_UTILS_H
#define SQLITE_UTILS_H

#include <string>
#include <vector>
#include "sqlite_import.h"

#include "types.h"
#include "db_types.h"
#include "schema_object.h"
#include "iprocess_system_api_adapter.h"

namespace DistributedDB {
enum class TransactType {
    DEFERRED,
    IMMEDIATE,
};

struct OpenDbProperties {
    std::string uri{};
    bool createIfNecessary = true;
    bool isMemDb = false;
    std::vector<std::string> sqls{};
    CipherType cipherType = CipherType::AES_256_GCM;
    CipherPassword passwd{};
    std::string schema = "";
    std::string subdir = "";
    SecurityOption securityOpt{};
    int conflictReslovePolicy = DEFAULT_LAST_WIN;
    bool createDirByStoreIdOnly = false;
};

class SQLiteUtils {
public:
    // Initialize the SQLiteUtils with the given properties.
    static int OpenDatabase(const OpenDbProperties &properties, sqlite3 *&db);

    // Check the statement and prepare the new if statement is null
    static int GetStatement(sqlite3 *db, const std::string &sql, sqlite3_stmt *&statement);

    // Bind the Text to the statement
    static int BindTextToStatement(sqlite3_stmt *statement, int index, const std::string &str);

    static int BindInt64ToStatement(sqlite3_stmt *statement, int index, int64_t value);

    // Bind the blob to the statement
    static int BindBlobToStatement(sqlite3_stmt *statement, int index, const std::vector<uint8_t> &value,
        bool permEmpty = true);

    // Reset the statement
    static void ResetStatement(sqlite3_stmt *&statement, bool isNeedFinalize, int &errCode);

    // Step the statement
    static int StepWithRetry(sqlite3_stmt *statement, bool isMemDb = false);

    // Bind the prefix key range
    static int BindPrefixKey(sqlite3_stmt *statement, int index, const Key &keyPrefix);

    static int BeginTransaction(sqlite3 *db, TransactType type = TransactType::DEFERRED);

    static int CommitTransaction(sqlite3 *db);

    static int RollbackTransaction(sqlite3 *db);

    static int ExecuteRawSQL(sqlite3 *db, const std::string &sql);

    static int SetKey(sqlite3 *db, CipherType type, const CipherPassword &passwd);

    static int GetColumnBlobValue(sqlite3_stmt *statement, int index, std::vector<uint8_t> &value);

    static int ExportDatabase(sqlite3 *db, CipherType type, const CipherPassword &passwd, const std::string &newDbName);

    static int ExportDatabase(const std::string &srcFile, CipherType type, const CipherPassword &srcPasswd,
        const std::string &targetFile, const CipherPassword &passwd);

    static int Rekey(sqlite3 *db, const CipherPassword &passwd);

    static int GetVersion(const OpenDbProperties &properties, int &version);

    static int GetVersion(sqlite3 *db, int &version);

    static int SetUserVer(const OpenDbProperties &properties, int version);

    static int SetUserVer(sqlite3 *db, int version);

    static int MapSQLiteErrno(int errCode);

    static int SaveSchema(const OpenDbProperties &properties);

    static int SaveSchema(sqlite3 *db, const std::string &strSchema);

    static int GetSchema(const OpenDbProperties &properties, std::string &strSchema);

    static int GetSchema(sqlite3 *db, std::string &strSchema);

    static int IncreaseIndex(sqlite3 *db, const IndexName &name, const IndexInfo &info, SchemaType type,
        uint32_t skipSize);

    static int ChangeIndex(sqlite3 *db, const IndexName &name, const IndexInfo &info, SchemaType type,
        uint32_t skipSize);

    static int DecreaseIndex(sqlite3 *db, const IndexName &name);

    static int RegisterJsonFunctions(sqlite3 *db);
    // Register the flatBufferExtract function if the schema is of flatBuffer-type(To be refactor)
    static int RegisterFlatBufferFunction(sqlite3 *db, const std::string &inSchema);

    static int GetDbSize(const std::string &dir, const std::string &dbName, uint64_t &size);

    static int ExplainPlan(sqlite3 *db, const std::string &execSql, bool isQueryPlan);

    static int AttachNewDatabase(sqlite3 *db, CipherType type, const CipherPassword &passwd,
        const std::string &attachDbAbsPath, const std::string &attachAsName = "backup");

    static int CreateMetaDatabase(const std::string &metaDbPath);
private:

    static int CreateDataBase(const OpenDbProperties &properties, sqlite3 *&dbTemp);

    static int SetBusyTimeout(sqlite3 *db, int timeout);

    static void JsonExtractByPath(sqlite3_context *ctx, int argc, sqlite3_value **argv);

    static void JsonExtractInnerFunc(sqlite3_context *ctx, const ValueObject &inValue, const FieldPath &inPath);

    static void FlatBufferExtractByPath(sqlite3_context *ctx, int argc, sqlite3_value **argv);

    static void FlatBufferExtractInnerFunc(sqlite3_context *ctx, const SchemaObject &schema, const RawValue &inValue,
        RawString inPath);

    static void ExtractReturn(sqlite3_context *ctx, FieldType type, const FieldValue &value);

    static void CalcHashKey(sqlite3_context *ctx, int argc, sqlite3_value **argv);

    static void GetSysCurrentTime(sqlite3_context *ctx, int argc, sqlite3_value **argv);

    static int SetDataBaseProperty(sqlite3 *db, const OpenDbProperties &properties,
        const std::vector<std::string> &sqls);

#ifndef OMIT_ENCRYPT
    static int SetCipherSettings(sqlite3 *db, CipherType type);

    static std::string GetCipherName(CipherType type);
#endif
};
};  // namespace DistributedDB

#endif // SQLITE_UTILS_H
