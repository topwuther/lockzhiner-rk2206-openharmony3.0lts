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

#ifndef MULTI_STORAGE_ENGINE_H
#define MULTI_STORAGE_ENGINE_H

#ifndef OMIT_MULTI_VER
#include "storage_engine.h"
#include "macro_utils.h"
#include "ikvdb.h"
#include "ikvdb_multi_ver_data_storage.h"
#include "ikvdb_commit_storage.h"
#include "multi_ver_kvdata_storage.h"

namespace DistributedDB {
class MultiVerStorageEngine : public StorageEngine {
public:
    MultiVerStorageEngine();
    ~MultiVerStorageEngine() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(MultiVerStorageEngine);

    int InitDatabases(IKvDB *kvDB, IKvDBMultiVerDataStorage *dataStorage,
        IKvDBCommitStorage *commitStorage, MultiVerKvDataStorage *kvDataStorage, const StorageEngineAttr &poolSize);

protected:
    int CreateNewExecutor(bool isWrite, StorageExecutor *&handle) override;

private:
    IKvDB *kvDB_;
    IKvDBMultiVerDataStorage *dataStorage_;
    IKvDBCommitStorage *commitStorage_;
    MultiVerKvDataStorage *kvDataStorage_;
};
} // namespace DistributedDB
#endif // MULTI_STORAGE_ENGINE_H
#endif