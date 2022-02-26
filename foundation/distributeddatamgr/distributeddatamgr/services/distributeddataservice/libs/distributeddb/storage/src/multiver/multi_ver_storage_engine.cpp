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

#ifndef OMIT_MULTI_VER
#include "multi_ver_storage_engine.h"

#include "multi_ver_storage_executor.h"

#include "db_errno.h"

namespace DistributedDB {
MultiVerStorageEngine::MultiVerStorageEngine()
    : kvDB_(nullptr),
      dataStorage_(nullptr),
      commitStorage_(nullptr),
      kvDataStorage_(nullptr)
{}

MultiVerStorageEngine::~MultiVerStorageEngine()
{
    kvDB_ = nullptr;
    dataStorage_ = nullptr;
    commitStorage_ = nullptr;
    kvDataStorage_ = nullptr;
}

int MultiVerStorageEngine::InitDatabases(IKvDB *kvDB, IKvDBMultiVerDataStorage *dataStorage,
    IKvDBCommitStorage *commitStorage, MultiVerKvDataStorage *kvDataStorage, const StorageEngineAttr &poolSize)
{
    if (StorageEngine::CheckEngineAttr(poolSize)) {
        return -E_INVALID_ARGS;
    }
    if (kvDB == nullptr || dataStorage == nullptr ||
        commitStorage == nullptr || kvDataStorage == nullptr) {
        return -E_INVALID_DB;
    }
    engineAttr_ = poolSize;
    kvDB_ = kvDB;
    dataStorage_ = dataStorage;
    commitStorage_ = commitStorage;
    kvDataStorage_ = kvDataStorage;
    return Init();
}

int MultiVerStorageEngine::CreateNewExecutor(bool isWrite, StorageExecutor *&handle)
{
    handle = new (std::nothrow) MultiVerStorageExecutor(kvDB_, dataStorage_, commitStorage_, kvDataStorage_, isWrite);
    if (handle == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    return E_OK;
}
}
#endif