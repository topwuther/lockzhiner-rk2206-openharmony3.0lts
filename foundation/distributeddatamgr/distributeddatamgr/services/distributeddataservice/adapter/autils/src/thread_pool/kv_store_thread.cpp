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

#define LOG_TAG "KvStoreThread"

#include "kv_store_thread_pool_impl.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
KvStoreThread::KvStoreThread(KvStoreThreadPool *threadPool)
    : pool_(threadPool)
{
    realThread_ = std::thread([&, threadPool]() {
        // this makes me unconfortable: this lambda capture 'this' by reference, and right after this call this object
        // is move-constructed, so when we call this in Run(), we are actually refer to the old object. we can still
        // use all its non-virtual function, but all arguments and virtual-function are not available.
        Run(threadPool);
    });
}

KvStoreThread::KvStoreThread(KvStoreThread &&thread)
    : pool_(thread.pool_)
{
    realThread_ = std::move(thread.realThread_);
}

void KvStoreThread::Run(KvStoreThreadPool *pool)
{
    if (pool == nullptr) {
        ZLOGW("input param is null.");
        return;
    }

    auto impl = reinterpret_cast<KvStoreThreadPoolImpl *>(pool);
    while (impl->IsRunning()) {
        impl->ScheduleTask()();
    }
    ZLOGW("stop");
}

void KvStoreThread::Join()
{
    realThread_.join();
}

KvStoreThread::~KvStoreThread()
{}
} // namespace DistributedKv
} // namespace OHOS
