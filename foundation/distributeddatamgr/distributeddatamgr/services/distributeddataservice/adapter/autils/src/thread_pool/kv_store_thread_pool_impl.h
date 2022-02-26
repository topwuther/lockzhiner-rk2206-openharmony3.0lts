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

#ifndef KV_STORE_THREAD_POOL_IMPL_H
#define KV_STORE_THREAD_POOL_IMPL_H

#include <mutex>
#include <list>
#include <condition_variable>
#include "kv_store_thread_pool.h"
#include "kv_store_thread.h"

namespace OHOS {
namespace DistributedKv {
class KvStoreThreadPoolImpl : public KvStoreThreadPool {
public:
    KvStoreThreadPoolImpl(int threadNum, bool startImmediately);
    KvStoreThreadPoolImpl() = delete;
    KvStoreThreadPoolImpl(const KvStoreThreadPoolImpl &) = delete;
    KvStoreThreadPoolImpl(KvStoreThreadPoolImpl &&) = delete;
    KvStoreThreadPoolImpl& operator=(const KvStoreThreadPoolImpl &) = delete;
    KvStoreThreadPoolImpl& operator=(KvStoreThreadPoolImpl &&) = delete;
    bool AddTask(KvStoreTask &&task) override;
    void Stop() final;
    KvStoreTask ScheduleTask();
    bool IsRunning() const;
    virtual ~KvStoreThreadPoolImpl();
private:
    std::mutex taskListMutex{};
    std::list<KvStoreTask> taskList{};
    std::condition_variable has_task{};
    std::list<KvStoreThread> threadList{};
    int threadNum;
    void Start();
    bool running = false;
};
} // namespace DistributedKv
} // namespace OHOS

#endif // KV_STORE_THREAD_POOL_IMPL_H
