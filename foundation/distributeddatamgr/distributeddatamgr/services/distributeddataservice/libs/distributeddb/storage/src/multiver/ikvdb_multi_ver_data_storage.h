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

#ifndef I_KV_DB_MULTI_VER_DATA_STORAGE_H
#define I_KV_DB_MULTI_VER_DATA_STORAGE_H

#ifndef OMIT_MULTI_VER
#include <string>
#include <vector>

#include "db_types.h"
#include "ikvdb_multi_ver_transaction.h"
#include "multi_ver_def.h"
#include "multi_ver_kv_entry.h"

namespace DistributedDB {
enum class KvDataType {
    KV_DATA_LOCAL,
    KV_DATA_SYNC_P2P,
};

struct UpdateVerTimeStamp {
    uint64_t timestamp = 0ull;
    bool isNeedUpdate = false;
};

class IKvDBMultiVerDataStorage {
public:
    struct Property final {
        std::string path;
        std::string identifierName;
        bool isNeedCreate = true;
        CipherType cipherType = CipherType::AES_256_GCM;
        CipherPassword passwd;
    };

    virtual ~IKvDBMultiVerDataStorage() {};
    virtual int Open(const Property &property) = 0;
    virtual int StartWrite(KvDataType dataType, IKvDBMultiVerTransaction *&transaction) = 0;
    virtual int CommitWritePhaseOne(IKvDBMultiVerTransaction *transaction,
        const UpdateVerTimeStamp &multiVerTimeStamp) = 0;
    virtual int RollbackWritePhaseOne(IKvDBMultiVerTransaction *transaction, const Version &versionInfo) = 0;
    virtual int RollbackWrite(IKvDBMultiVerTransaction *transaction) = 0;
    virtual void CommitWritePhaseTwo(IKvDBMultiVerTransaction *transaction) = 0;
    virtual IKvDBMultiVerTransaction *StartRead(KvDataType dataType, const Version &versionInfo, int &errCode) = 0;
    virtual void ReleaseTransaction(IKvDBMultiVerTransaction *transaction) = 0;
    virtual void Close() = 0;
    virtual int CheckVersion(const Property &property, bool &isDbExist) const = 0;
    virtual int GetVersion(const Property &property, int &version, bool &isDbExisted) const = 0;
    virtual int BackupCurrentDatabase(const Property &property, const std::string &dir) = 0;
    virtual int ImportDatabase(const Property &property, const std::string &dir, const CipherPassword &passwd) = 0;
};
} // namespace DistributedDB

#endif  // I_KV_DB_MULTI_VER_DATA_STORAGE_H
#endif