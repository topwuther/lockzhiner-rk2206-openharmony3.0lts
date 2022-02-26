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

#include "task_pool.h"
#include "db_errno.h"
#include "log_print.h"
#include "task_pool_impl.h"

namespace DistributedDB {
TaskPool *TaskPool::Create(int maxThreads, int minThreads, int &errCode)
{
    TaskPool *taskPool = new (std::nothrow) TaskPoolImpl(maxThreads, minThreads);
    if (taskPool == nullptr) {
        LOGE("alloc task pool failed.");
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    errCode = E_OK;
    return taskPool;
}

void TaskPool::Release(TaskPool *&taskPool)
{
    if (taskPool != nullptr) {
        delete taskPool;
        taskPool = nullptr;
    }
}
} // namespace DistributedDB
