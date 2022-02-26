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

#ifndef MULTI_VER_KV_DATA_STORAGE_H
#define MULTI_VER_KV_DATA_STORAGE_H

#ifndef OMIT_MULTI_VER
#include <mutex>
#include <string>

#include "ikvdb.h"
#include "macro_utils.h"

namespace DistributedDB {
class SliceTransaction {
public:
    SliceTransaction(bool isWrite, IKvDBConnection *connect);
    ~SliceTransaction();
    int Close();
    int PutData(const Key &key, const Value &value, bool isAddCount);
    int GetData(const Key &key, Value &value) const;
    int DeleteData(const Key &key);
    int StartTransaction();
    int CommitTransaction();
    int RollbackTransaction();
private:
    bool isWrite_;
    IKvDBConnection *connect_;
};

class MultiVerKvDataStorage {
public:
    struct Property final {
        std::string dataDir;
        std::string identifierName;
        bool isNeedCreate = true;
        CipherType cipherType = CipherType::AES_256_GCM;
        CipherPassword passwd;
    };

    MultiVerKvDataStorage();
    ~MultiVerKvDataStorage();

    DISABLE_COPY_ASSIGN_MOVE(MultiVerKvDataStorage);

    int Open(const Property &property);

    void Close();

    int PutMetaData(const Key &key, const Value &value);

    int GetMetaData(const Key &key, Value &value) const;

    SliceTransaction *GetSliceTransaction(bool isWrite, int &errCode);

    void ReleaseSliceTransaction(SliceTransaction *&transaction);

    int RunRekeyLogic(CipherType type, const CipherPassword &passwd);

    int RunExportLogic(CipherType type, const CipherPassword &passwd, const std::string &dbDir) const;

    int CheckVersion(const Property &property, bool &isDbAllExist) const;

    int BackupCurrentDatabase(const Property &property, const std::string &dir);

    int ImportDatabase(const Property &property, const std::string &dir, const CipherPassword &passwd);

private:
    int GetVersion(const Property &property, int &metaVer, bool &isMetaDbExist,
        int &sliceVer, bool &isSliceDbExist) const;

    IKvDB *kvStorage_;
    IKvDB *metaStorage_;
    IKvDBConnection *kvStorageConnection_;
    IKvDBConnection *metaStorageConnection_;
    mutable std::mutex metaDataMutex_;
    mutable std::mutex kvDataMutex_;
};
}

#endif // MULTI_VER_KV_DATA_STORAGE_H
#endif
