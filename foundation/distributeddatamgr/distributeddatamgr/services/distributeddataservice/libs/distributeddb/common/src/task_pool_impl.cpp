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
#include "task_pool_impl.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
constexpr int TaskPoolImpl::IDLE_WAIT_PERIOD;

TaskPoolImpl::TaskPoolImpl(int maxThreads, int minThreads)
    : genericTasks_(false),
      genericTaskCount_(0),
      queuedTaskCount_(0),
      isStarted_(false),
      isStopping_(false),
      maxThreads_(maxThreads),
      minThreads_(minThreads),
      curThreads_(0),
      idleThreads_(0)
{}

TaskPoolImpl::~TaskPoolImpl()
{}

int TaskPoolImpl::Start()
{
    if (maxThreads_ < minThreads_) {
        LOGE("Start task pool failed, maxThreads(%d) < minThreads(%d).",
            maxThreads_, minThreads_);
        return -E_INVALID_ARGS;
    }
    if (maxThreads_ <= 0) {
        LOGE("Start task pool failed, maxThreads(%d) <= 0.", maxThreads_);
        return -E_INVALID_ARGS;
    }
    if (minThreads_ < 0) {
        LOGE("Start task pool failed, minThreads(%d) < 0.", minThreads_);
        return -E_INVALID_ARGS;
    }
    LOGI("Start task pool min:%d, max:%d", minThreads_, maxThreads_);
    std::lock_guard<std::mutex> guard(tasksMutex_);
    isStarted_ = true; // parameters checked ok.
    isStopping_ = false;
    int errCode = SpawnThreads(true);
    if (errCode != E_OK) {
        LOGW("Spawn threads failed when starting the task pool.");
        // ignore the error, we will try when schedule().
    }
    return E_OK;
}

void TaskPoolImpl::Stop()
{
    std::unique_lock<std::mutex> lock(tasksMutex_);
    if (!isStarted_) {
        return;
    }
    isStopping_ = true;
    hasTasks_.notify_all();
    allThreadsExited_.wait(lock, [this]() {
        return this->curThreads_ <= 0;
    });
    isStarted_ = false;
}

int TaskPoolImpl::Schedule(const Task &task)
{
    if (!task) {
        return -E_INVALID_ARGS;
    }
    std::lock_guard<std::mutex> guard(tasksMutex_);
    if (!isStarted_) {
        LOGE("Schedule failed, the task pool is not started.");
        return -E_NOT_PERMIT;
    }
    if (isStopping_) {
        LOGI("Schedule failed, the task pool is stopping.");
        return -E_STALE;
    }
    genericTasks_.PutTask(task);
    ++genericTaskCount_;
    hasTasks_.notify_one();
    TryToSpawnThreads();
    return E_OK;
}

int TaskPoolImpl::Schedule(const std::string &queueTag, const Task &task)
{
    if (!task) {
        return -E_INVALID_ARGS;
    }
    std::lock_guard<std::mutex> guard(tasksMutex_);
    if (!isStarted_) {
        LOGE("Schedule failed, the task pool is not started.");
        return -E_NOT_PERMIT;
    }
    if (isStopping_) {
        LOGI("Schedule failed, the task pool is stopping.");
        return -E_STALE;
    }
    queuedTasks_[queueTag].PutTask(task);
    ++queuedTaskCount_;
    hasTasks_.notify_all();
    TryToSpawnThreads();
    return E_OK;
}

void TaskPoolImpl::ShrinkMemory(const std::string &tag)
{
    std::lock_guard<std::mutex> guard(tasksMutex_);
    auto iter = queuedTasks_.find(tag);
    if (iter != queuedTasks_.end()) {
        if (iter->second.IsEmptyAndUnlocked()) {
            queuedTasks_.erase(iter);
        }
    }
}

bool TaskPoolImpl::IdleExit(std::unique_lock<std::mutex> &lock)
{
    if (isStopping_) {
        return true;
    }
    ++idleThreads_;
    bool isGenericWorker = IsGenericWorker();
    if (!isGenericWorker && (curThreads_ > minThreads_)) {
        std::cv_status status = hasTasks_.wait_for(lock,
            std::chrono::seconds(IDLE_WAIT_PERIOD));
        if (status == std::cv_status::timeout &&
            genericTaskCount_ <= 0) {
            --idleThreads_;
            return true;
        }
    } else {
        if (isGenericWorker) {
            hasTasks_.notify_all();
        }
        hasTasks_.wait(lock);
    }
    --idleThreads_;
    return false;
}

void TaskPoolImpl::SetThreadFree()
{
    for (auto &pair : queuedTasks_) {
        TaskQueue *tq = &pair.second;
        tq->ReleaseLock();
    }
}

Task TaskPoolImpl::ReapTask(TaskQueue *&queue)
{
    Task task = genericTasks_.GetTaskAutoLock();
    if (task != nullptr) {
        queue = nullptr;
        return task;
    }

    queue = nullptr;
    if (IsGenericWorker() && (curThreads_ > 1)) { // 1 indicates self.
        SetThreadFree();
        return nullptr;
    }
    for (auto &pair : queuedTasks_) {
        TaskQueue *tq = &pair.second;
        task = tq->GetTaskAutoLock();
        if (task != nullptr) {
            queue = tq;
            return task;
        }
    }
    return nullptr;
}

int TaskPoolImpl::GetTask(Task &task, TaskQueue *&queue)
{
    std::unique_lock<std::mutex> lock(tasksMutex_);

    while (true) {
        task = ReapTask(queue);
        if (task != nullptr) {
            return E_OK;
        }

        if (IdleExit(lock)) {
            break;
        }
    }
    return E_OK;
}

int TaskPoolImpl::SpawnThreads(bool isStart)
{
    if (!isStarted_) {
        LOGE("Spawn task pool threads failed, pool is not started.");
        return -E_NOT_PERMIT;
    }
    if (curThreads_ >= maxThreads_) {
        // the pool is full of threads.
        return E_OK;
    }

    int limits = isStart ? minThreads_ : (curThreads_ + 1);
    while (curThreads_ < limits) {
        ++curThreads_;
        std::thread thread([this]() {
            TaskWorker();
        });
        LOGI("Task pool spawn cur:%d idle:%d.", curThreads_, idleThreads_);
        thread.detach();
    }
    return E_OK;
}

bool TaskPoolImpl::IsGenericWorker() const
{
    return genericThread_ == std::this_thread::get_id();
}

void TaskPoolImpl::BecomeGenericWorker()
{
    std::lock_guard<std::mutex> guard(tasksMutex_);
    if (genericThread_ == std::thread::id()) {
        genericThread_ = std::this_thread::get_id();
    }
}

void TaskPoolImpl::ExitWorker()
{
    std::lock_guard<std::mutex> guard(tasksMutex_);
    if (IsGenericWorker()) {
        genericThread_ = std::thread::id();
    }
    --curThreads_;
    allThreadsExited_.notify_all();
    LOGI("Task pool thread exit, cur:%d idle:%d, genericTaskCount:%d, queuedTaskCount:%d.",
        curThreads_, idleThreads_, genericTaskCount_, queuedTaskCount_);
}

void TaskPoolImpl::TaskWorker()
{
    BecomeGenericWorker();

    while (true) {
        TaskQueue *taskQueue = nullptr;
        Task task = nullptr;

        int errCode = GetTask(task, taskQueue);
        if (errCode != E_OK) {
            LOGE("Thread worker gets task failed, err:'%d'.", errCode);
            break;
        }
        if (task == nullptr) {
            // Idle thread exit.
            break;
        }

        task();
        FinishExecuteTask(taskQueue);
    }

    ExitWorker();
}

void TaskPoolImpl::FinishExecuteTask(TaskQueue *taskQueue)
{
    std::lock_guard<std::mutex> guard(tasksMutex_);
    if (taskQueue != nullptr) {
        taskQueue->ReleaseLock();
        --queuedTaskCount_;
    } else {
        --genericTaskCount_;
    }
}

void TaskPoolImpl::TryToSpawnThreads()
{
    if ((curThreads_ >= maxThreads_) ||
        (curThreads_ >= (queuedTaskCount_ + genericTaskCount_))) {
        return;
    }
    (void)(SpawnThreads(false));
}
} // namespace DistributedDB
