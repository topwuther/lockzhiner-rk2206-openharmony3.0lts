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

#include "default_factory.h"

#include <string>

#include "db_errno.h"
#include "sqlite_local_kvdb.h"
#ifndef OMIT_MULTI_VER
#include "multi_ver_natural_store.h"
#include "multi_ver_natural_store_commit_storage.h"
#endif
#include "sqlite_single_ver_natural_store.h"
#ifndef OMIT_MULTI_VER
#include "sqlite_multi_ver_data_storage.h"
#endif

namespace DistributedDB {
IKvDB *DefaultFactory::CreateKvDb(KvDBType kvDbType, int &errCode)
{
    switch (kvDbType) {
        case LOCAL_KVDB:
            return CreateLocalKvDB(errCode);
        case SINGER_VER_KVDB:
            return CreateSingleVerNaturalStore(errCode);
#ifndef OMIT_MULTI_VER
        case MULTI_VER_KVDB:
            return CreateMultiVerNaturalStore(errCode);
#endif
        default:
            errCode = -E_INVALID_ARGS;
            return nullptr;
    }
}

IKvDB *DefaultFactory::CreateLocalKvDB(int &errCode)
{
    IKvDB *kvDb = new (std::nothrow) SQLiteLocalKvDB();
    errCode = ((kvDb == nullptr) ? -E_OUT_OF_MEMORY : E_OK);
    return kvDb;
}

#ifndef OMIT_MULTI_VER
// Create the multi-version natural store, it contains a commit version and commit storage kvdb.
IKvDB *DefaultFactory::CreateMultiVerNaturalStore(int &errCode)
{
    IKvDB *kvDb = new (std::nothrow) MultiVerNaturalStore();
    errCode = ((kvDb == nullptr) ? -E_OUT_OF_MEMORY : E_OK);
    return kvDb;
}
#endif

// Create the single version natural store.
IKvDB *DefaultFactory::CreateSingleVerNaturalStore(int &errCode)
{
    IKvDB *kvDb = new (std::nothrow) SQLiteSingleVerNaturalStore();
    errCode = ((kvDb == nullptr) ? -E_OUT_OF_MEMORY : E_OK);
    return kvDb;
}

// Create a key-value database for commit storage module.
IKvDB *DefaultFactory::CreateCommitStorageDB(int &errCode)
{
    return CreateLocalKvDB(errCode);
}

#ifndef OMIT_MULTI_VER
IKvDBMultiVerDataStorage *DefaultFactory::CreateMultiVerStorage(int &errCode)
{
    IKvDBMultiVerDataStorage *multiStorage = new (std::nothrow) SQLiteMultiVerDataStorage();
    errCode = ((multiStorage == nullptr) ? -E_OUT_OF_MEMORY : E_OK);
    return multiStorage;
}

// Create the commit storage. The object can be deleted directly when it is not needed.
IKvDBCommitStorage *DefaultFactory::CreateMultiVerCommitStorage(int &errCode)
{
    IKvDBCommitStorage *commitStorage = new (std::nothrow) MultiVerNaturalStoreCommitStorage();
    errCode = ((commitStorage == nullptr) ? -E_OUT_OF_MEMORY : E_OK);
    return commitStorage;
}
#endif
} // namespace DistributedDB
