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

#include "semaphore.h"

#include <functional>

namespace DistributedDB {
using std::unique_lock;
using std::mutex;
using std::condition_variable;

Semaphore::Semaphore(int count)
    : count_(count)
{}

Semaphore::~Semaphore()
{}

bool Semaphore::WaitSemaphore(int waitSecond)
{
    unique_lock<mutex> lock(lockMutex_);
    bool result = cv_.wait_for(lock, std::chrono::seconds(waitSecond),
        std::bind(&Semaphore::CompareCount, this));
    if (result == true) {
        --count_;
    }
    return result;
}

void Semaphore::WaitSemaphore()
{
    unique_lock<mutex> lock(lockMutex_);
    cv_.wait(lock, std::bind(&Semaphore::CompareCount, this));
    --count_;
}

void Semaphore::SendSemaphore()
{
    unique_lock<mutex> lock(lockMutex_);
    count_++;
    cv_.notify_one();
}

bool Semaphore::CompareCount() const
{
    return count_ > 0;
}
}
