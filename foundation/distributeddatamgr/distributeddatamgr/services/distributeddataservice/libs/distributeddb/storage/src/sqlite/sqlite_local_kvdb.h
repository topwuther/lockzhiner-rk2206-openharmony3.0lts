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

#ifndef SQLITE_LOCAL_KV_DB_H
#define SQLITE_LOCAL_KV_DB_H

#include <string>
#include <mutex>

#include "local_kvdb.h"
#include "sqlite_local_storage_executor.h"
#include "sqlite_storage_engine.h"

namespace DistributedDB {
class SQLiteLocalKvDB final : public LocalKvDB {
public:
    SQLiteLocalKvDB();
    ~SQLiteLocalKvDB() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteLocalKvDB);

    // Save the option and uri for sqlite
    int Open(const KvDBProperties &kvDBProp) override;

    // Create a connection object.
    GenericKvDBConnection *NewConnection(int &errCode) override;

    // Invoked automatically when connection count is zero
    void Close() override;

    int Rekey(const CipherPassword &passwd) override;

    int Export(const std::string &filePath, const CipherPassword &passwd) override;

    int Import(const std::string &filePath, const CipherPassword &passwd) override;

    int RunExportLogic(CipherType type, const CipherPassword &passwd, const std::string &newDbName);

    int RunRekeyLogic(CipherType type, const CipherPassword &passwd);

    SQLiteLocalStorageExecutor *GetHandle(bool isWrite, int &errCode,
        OperatePerm perm = OperatePerm::NORMAL_PERM) const;

    void ReleaseHandle(SQLiteLocalStorageExecutor *&handle) const;

    int GetVersion(const KvDBProperties &kvDBProp, int &version, bool &isDbExisted) const;

    int SetVersion(const KvDBProperties &kvDBProp, int version);

    const KvDBProperties &GetDbProperties() const;

    KvDBProperties &GetDbPropertyForUpdate();

    static int BackupCurrentDatabase(const KvDBProperties &properties, const std::string &dir);

    static int ImportDatabase(const KvDBProperties &properties, const std::string &dir, const CipherPassword &passwd);

    int RemoveKvDB(const KvDBProperties &properties) override;

    int GetKvDBSize(const KvDBProperties &properties, uint64_t &size) const override;

    int InitDatabaseContext(const KvDBProperties &kvDBProp);

    void EnableAutonomicUpgrade() override;

private:
    int InitStorageEngine(const KvDBProperties &kvDBProp);

    void InitDataBaseOption(const KvDBProperties &kvDBProp, OpenDbProperties &option) const;

    int CheckVersionAndUpgradeIfNeed(const OpenDbProperties &openProp);

    DECLARE_OBJECT_TAG(SQLiteLocalKvDB);

    bool isAutonomicUpgradeEnable_ = false;
    SQLiteStorageEngine *storageEngine_;
};
} // namespace DistributedDB

#endif  // SQLITE_LOCAL_KV_DB_H
