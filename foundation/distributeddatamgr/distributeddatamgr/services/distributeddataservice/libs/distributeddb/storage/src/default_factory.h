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

#ifndef DEFAULT_FACTORY_H
#define DEFAULT_FACTORY_H

#include "ikvdb.h"
#include "ikvdb_factory.h"

namespace DistributedDB {
class DefaultFactory final : public IKvDBFactory {
public:
    DefaultFactory() {}
    ~DefaultFactory() override {}

    DISABLE_COPY_ASSIGN_MOVE(DefaultFactory);
    IKvDB *CreateKvDb(KvDBType kvDbType, int &errCode) override;

    // Create a key-value database for commit storage module.
    IKvDB *CreateCommitStorageDB(int &errCode) override;

#ifndef OMIT_MULTI_VER
    // Create the multi version storage for multi version natural store
    IKvDBMultiVerDataStorage *CreateMultiVerStorage(int &errCode) override;

    // Create the commit storage. The object can be deleted directly when it is not needed.
    IKvDBCommitStorage *CreateMultiVerCommitStorage(int &errCode) override;
#endif
private:
    // Create the a local kv db
    IKvDB *CreateLocalKvDB(int &errCode);

#ifndef OMIT_MULTI_VER
    // Create the a natural store, it contains a commit version and commit storage kvdb.
    IKvDB *CreateMultiVerNaturalStore(int &errCode);
#endif

    IKvDB *CreateSingleVerNaturalStore(int &errCode);
};
} // namespace DistributedDB
#endif // DEFAULT_FACTORY_H
