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

#ifndef SQLITE_KV_DB_MULTI_VER_DATA_STORAGE_H
#define SQLITE_KV_DB_MULTI_VER_DATA_STORAGE_H

#ifndef OMIT_MULTI_VER
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <thread>
#include <condition_variable>

#include "db_types.h"
#include "kvdb_properties.h"
#include "ikvdb_multi_ver_data_storage.h"
#include "macro_utils.h"
#include "multi_ver_value_object.h"
#include "generic_multi_ver_kv_entry.h"
#include "sqlite_multi_ver_transaction.h"

namespace DistributedDB {
class SQLiteMultiVerDataStorage : public IKvDBMultiVerDataStorage {
public:
    SQLiteMultiVerDataStorage();
    ~SQLiteMultiVerDataStorage() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteMultiVerDataStorage);

    int Open(const Property &property) override;

    int StartWrite(KvDataType dataType, IKvDBMultiVerTransaction *&transaction) override;

    int CommitWritePhaseOne(IKvDBMultiVerTransaction *transaction,
        const UpdateVerTimeStamp &multiVerTimeStamp) override;

    int RollbackWritePhaseOne(IKvDBMultiVerTransaction *transaction, const Version &versionInfo) override;

    int RollbackWrite(IKvDBMultiVerTransaction *transaction) override;

    void CommitWritePhaseTwo(IKvDBMultiVerTransaction *transaction) override;

    IKvDBMultiVerTransaction *StartRead(KvDataType dataType, const Version &versionInfo, int &errCode) override;

    void ReleaseTransaction(IKvDBMultiVerTransaction *transaction) override;

    void Close() override;

    int RunRekeyLogic(CipherType type, const CipherPassword &passwd);

    int RunExportLogic(CipherType type, const CipherPassword &passwd, const std::string &dbDir);

    int CheckVersion(const Property &property, bool &isDbExist) const override;

    int GetVersion(const Property &property, int &version, bool &isDbExisted) const override;

    int BackupCurrentDatabase(const Property &property, const std::string &dir) override;

    int ImportDatabase(const Property &property, const std::string &dir, const CipherPassword &passwd) override;

private:
    Property property_;
    std::string uri_;
    std::map<IKvDBMultiVerTransaction *, bool> readTransactions_;
    SQLiteMultiVerTransaction *writeTransaction_;
    bool writeTransactionUsed_;
    std::mutex transactionMutex_;
    std::condition_variable readCondition_;
    std::condition_variable writeCondition_;
    std::thread::id writeHolderId_;
};
} // namespace DistributedDB

#endif // SQLITE_KV_DB_MULTI_VER_DATA_STORAGE_H
#endif