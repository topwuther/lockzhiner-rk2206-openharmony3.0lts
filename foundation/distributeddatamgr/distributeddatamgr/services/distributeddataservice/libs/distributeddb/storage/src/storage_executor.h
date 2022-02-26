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

#ifndef STORAGE_EXECUTOR_H
#define STORAGE_EXECUTOR_H

#include "macro_utils.h"

namespace DistributedDB {
enum EngineState {
    INVALID = -1, // default value, representative database is not generated
    CACHEDB,
    ATTACHING, // main db and cache db attach together
    MIGRATING, // begine to Migrate data
    MAINDB,
    ENGINE_BUSY, // In order to change handle during the migration process, it is temporarily unavailable
};

class StorageExecutor {
public:
    explicit StorageExecutor(bool writable);
    virtual ~StorageExecutor();

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(StorageExecutor);

    virtual bool GetWritable() const;

    virtual int CheckCorruptedStatus(int errCode) const;

    virtual bool GetCorruptedStatus() const;

    virtual void SetCorruptedStatus() const;

    virtual int Reset() = 0;

protected:
    bool writable_;
    mutable bool isCorrupted_;
};
} // namespace DistributedDB

#endif // STORAGE_EXECUTOR_H
