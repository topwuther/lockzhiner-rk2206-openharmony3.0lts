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

#ifndef SYNC_TARGET_H
#define SYNC_TARGET_H

#include "isync_target.h"

namespace DistributedDB {
class SyncTarget : public ISyncTarget {
public:
    SyncTarget() : operation_(nullptr), taskType_(0), mode_(0) {};
    virtual ~SyncTarget();

    // Get the Sync Id of this task
    int GetSyncId() const override;

    // Set the type of this task request or response
    void SetTaskType(int taskType) override;

    // Get the type of this task request or response
    int GetTaskType() const override;

    // Set the mode of this task request or response
    void SetMode(int mode) override;

    // Get the mode of this task request or response
    int GetMode() const override;

    // Set a Sync Status, it will increase the ref of operation
    void SetSyncOperation(SyncOperation *operation) override;

    // Get a SyncOperation
    void GetSyncOperation(SyncOperation *&operation) const override;

    // Is this target is a auto sync
    bool IsAutoSync() const override;

protected:
    SyncOperation *operation_;
    int taskType_; // sync task or response task;
    int mode_;
};
} // namespace DistributedDB

#endif // SYNC_TARGET_H
