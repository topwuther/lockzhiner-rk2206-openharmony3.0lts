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

#ifndef KV_STORE_THREAD_H
#define KV_STORE_THREAD_H

#include <thread>
#include "kv_store_thread_pool.h"

namespace OHOS {
namespace DistributedKv {
class KvStoreThread {
public:
    explicit KvStoreThread(KvStoreThreadPool *threadPool);
    KvStoreThread(KvStoreThread &&thread);
    KvStoreThread(const KvStoreThread &) = delete;
    KvStoreThread &operator=(KvStoreThread &&) = delete;
    KvStoreThread &operator=(const KvStoreThread &) = delete;
    void Run(KvStoreThreadPool *threadPool);
    void Join();
    ~KvStoreThread();
private:
    KvStoreThreadPool *pool_;
    std::thread realThread_;
};
} // namespace DistributedKv
} // namespace OHOS

#endif // KV_STORE_THREAD_H
