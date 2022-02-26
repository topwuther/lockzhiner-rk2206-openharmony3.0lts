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

#define LOG_TAG "KvStoreThreadPoolImpl"

#include "kv_store_thread_pool_impl.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
KvStoreThreadPoolImpl::~KvStoreThreadPoolImpl()
{
    Stop();
}

void KvStoreThreadPoolImpl::Start()
{
    ZLOGI("start");
    running = true;
    for (int i = 0; i < threadNum; i++) {
        threadList.push_back(KvStoreThread(this));
    }
}

void KvStoreThreadPoolImpl::Stop()
{
    ZLOGW("stop");
    if (!running) {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(taskListMutex);
        running = false;
        for (auto task = taskList.begin(); task != taskList.end(); task++) {
            ZLOGI("running task in stop()");
            (*task)();
            ZLOGI("running task finish");
        }
        taskList.clear();
    }
    has_task.notify_all();
    for (auto thread = threadList.begin(); thread != threadList.end(); thread++) {
        thread->Join();
    }
}

bool KvStoreThreadPoolImpl::IsRunning() const
{
    return running;
}

KvStoreThreadPoolImpl::KvStoreThreadPoolImpl(int threadNum, bool startImmediately)
    : taskList(), threadList(), threadNum(threadNum)
{
    if (threadNum <= 0 || threadNum > MAX_POOL_SIZE) {
        this->threadNum = DEFAULT_POOL_SIZE;
    }
    if (startImmediately) {
        Start();
    }
}

bool KvStoreThreadPoolImpl::AddTask(KvStoreTask &&task)
{
    ZLOGD("start");
    if (threadList.empty()) {
        Start();
    }
    std::unique_lock<std::mutex> lock(taskListMutex);
    if (!running) {
        return false;
    }
    taskList.push_back(std::move(task));
    has_task.notify_one();
    return true;
}

KvStoreTask KvStoreThreadPoolImpl::ScheduleTask()
{
    std::unique_lock<std::mutex> lock(taskListMutex);
    if (taskList.empty() && running) {
        has_task.wait(lock, [&]() {return !running || !taskList.empty(); });
    }
    if (taskList.empty()) {
        ZLOGW("taskList empty. schedule empty task(pool stopping?)");
        return KvStoreTask([]() {;});
    }
    KvStoreTask ret = std::move(taskList.front());
    taskList.pop_front();
    return ret;
}
} // namespace DistributedKv
} // namespace OHOS
