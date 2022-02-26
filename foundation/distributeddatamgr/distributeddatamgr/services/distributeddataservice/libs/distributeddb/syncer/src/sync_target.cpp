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

#include "sync_target.h"

#include "db_errno.h"
#include "sync_operation.h"
#include "log_print.h"

namespace DistributedDB {
SyncTarget::~SyncTarget()
{
    operation_ = nullptr;
}

int SyncTarget::GetSyncId() const
{
    if (operation_ == nullptr) {
        return 0;
    }
    return operation_->GetSyncId();
}

void SyncTarget::SetTaskType(int taskType)
{
    taskType_ = taskType;
}

int SyncTarget::GetTaskType() const
{
    return taskType_;
}

void SyncTarget::SetMode(int mode)
{
    mode_ = mode;
}

int SyncTarget::GetMode() const
{
    return mode_;
}

void SyncTarget::SetSyncOperation(SyncOperation *operation)
{
    if ((operation != nullptr) && !operation->IsKilled()) {
        operation_ = operation;
        mode_ = operation->GetMode();
        taskType_ = REQUEST;
    }
    operation_ = operation;
}

void SyncTarget::GetSyncOperation(SyncOperation *&operation) const
{
    if (operation_ == nullptr) {
        LOGD("GetSyncOperation is nullptr");
    }
    operation = operation_;
}

bool SyncTarget::IsAutoSync() const
{
    if (operation_ == nullptr) {
        return false;
    }
    return operation_->IsAutoSync();
}
} // namespace DistributedDB

