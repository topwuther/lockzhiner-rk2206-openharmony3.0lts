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

#ifndef TASK_POOL_H
#define TASK_POOL_H

#include <string>
#include <functional>
#include "macro_utils.h"

namespace DistributedDB {
using Task = std::function<void(void)>;

class TaskPool {
public:
    // Start the task pool.
    virtual int Start() = 0;

    // Stop the task pool.
    virtual void Stop() = 0;

    // Schedule a task, the task can be ran in any thread.
    virtual int Schedule(const Task &task) = 0;

    // Schedule tasks using FIFO policy(tasks with the same 'tag').
    virtual int Schedule(const std::string &tag, const Task &task) = 0;

    // Shrink memory associated with the given tag if possible.
    virtual void ShrinkMemory(const std::string &tag) = 0;

    // Create/Destroy a task pool.
    static TaskPool *Create(int maxThreads, int minThreads, int &errCode);
    static void Release(TaskPool *&taskPool);

protected:
    TaskPool() = default;
    virtual ~TaskPool() {}
    DISABLE_COPY_ASSIGN_MOVE(TaskPool);
};
} // namespace DistributedDB

#endif // TASK_POOL_H