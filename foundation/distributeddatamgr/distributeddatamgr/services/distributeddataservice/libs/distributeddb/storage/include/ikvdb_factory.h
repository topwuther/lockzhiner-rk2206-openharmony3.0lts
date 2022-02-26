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

#ifndef I_KV_DB_FACTORY_H
#define I_KV_DB_FACTORY_H

#include <mutex>

#include "ikvdb.h"
#include "db_types.h"
#ifndef OMIT_MULTI_VER
#include "ikvdb_multi_ver_data_storage.h"
#include "ikvdb_commit_storage.h"
#endif

namespace DistributedDB {
enum KvDBType {
    LOCAL_KVDB = 0,
    SINGER_VER_KVDB,
    MULTI_VER_KVDB,
    UNSUPPORT_KVDB_TYPE,
};

class IKvDBFactory {
public:
    virtual ~IKvDBFactory() {}

    // Get current factory object.
    static IKvDBFactory *GetCurrent();

    // Set the factory object to 'current'
    static void Register(IKvDBFactory *factory);

    virtual IKvDB *CreateKvDb(KvDBType kvDbType, int &errCode) = 0;

    // Create a key-value database for commit storage module.
    virtual IKvDB *CreateCommitStorageDB(int &errCode) = 0;

#ifndef OMIT_MULTI_VER
    // Create the multi version storage for multi version natural store
    virtual IKvDBMultiVerDataStorage *CreateMultiVerStorage(int &errCode) = 0;

    // Create the commit storage. The object can be deleted directly when it is not needed.
    virtual IKvDBCommitStorage *CreateMultiVerCommitStorage(int &errCode) = 0;
#endif
private:
    static IKvDBFactory *factory_;
    static std::mutex instanceLock_;
};
} // namespace DistributedDB

#endif // I_KV_DB_FACTORY_H
