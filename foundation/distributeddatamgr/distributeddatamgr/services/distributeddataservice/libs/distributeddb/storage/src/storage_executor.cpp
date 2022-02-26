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

#include "storage_executor.h"

#include "db_errno.h"

namespace DistributedDB {
StorageExecutor::StorageExecutor(bool writable)
    : writable_(writable),
      isCorrupted_(false)
{}

StorageExecutor::~StorageExecutor()
{}

bool StorageExecutor::GetWritable() const
{
    return writable_;
}

bool StorageExecutor::GetCorruptedStatus() const
{
    return isCorrupted_;
}

void StorageExecutor::SetCorruptedStatus() const
{
    isCorrupted_ = true;
}

int StorageExecutor::CheckCorruptedStatus(int errCode) const
{
    if (errCode == -E_INVALID_PASSWD_OR_CORRUPTED_DB) {
        SetCorruptedStatus();
    }
    return errCode;
}
} // namespace DistributedDB
