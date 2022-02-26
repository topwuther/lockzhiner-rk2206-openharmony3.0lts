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

#ifndef I_SYNC_TARGET_H
#define I_SYNC_TARGET_H

#include "sync_operation.h"

namespace DistributedDB {
class ISyncTarget {
public:
    enum TaskType {
        REQUEST = 1,
        RESPONSE
    };

    virtual ~ISyncTarget() {};

    // Get the Sync Id of this task
    virtual int GetSyncId() const = 0;

    // Set the type of this task request or response
    virtual void SetTaskType(int taskType) = 0;

    // Get the type of this task request or response
    virtual int GetTaskType() const = 0;

    // Set the mode of this task request or response
    virtual void SetMode(int mode) = 0;

    // Get the mode of this task request or response
    virtual int GetMode() const = 0;

    // Set a Sync Status, it will increase the ref of operation
    virtual void SetSyncOperation(SyncOperation *operation) = 0;

    // Get a SyncOperation
    virtual void GetSyncOperation(SyncOperation *&operation) const = 0;

    // Is this target is a auto sync
    virtual bool IsAutoSync() const = 0;
};
} // namespace DistributedDB

#endif // I_SYNC_TARGET_H
