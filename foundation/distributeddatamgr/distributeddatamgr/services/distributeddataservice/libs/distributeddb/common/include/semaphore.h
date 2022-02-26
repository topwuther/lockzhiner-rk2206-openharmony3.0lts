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

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include "macro_utils.h"

namespace DistributedDB {
class Semaphore {
public:
    explicit Semaphore(int count);
    ~Semaphore();

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(Semaphore);

    bool WaitSemaphore(int waitSecond);

    void WaitSemaphore();

    void SendSemaphore();

private:
    bool CompareCount() const;
    std::mutex lockMutex_;
    std::condition_variable cv_;
    int count_;
};
}

#endif
