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

#ifndef KV_STORE_THREAD_POOL_H
#define KV_STORE_THREAD_POOL_H

#include <memory>
#include "kv_store_task.h"

namespace OHOS {
namespace DistributedKv {
class KvStoreThreadPool {
public:
    KvStoreThreadPool(KvStoreThreadPool &&) = delete;
    KvStoreThreadPool(const KvStoreThreadPool &) = delete;
    KvStoreThreadPool &operator=(KvStoreThreadPool &&) = delete;
    KvStoreThreadPool &operator=(const KvStoreThreadPool &) = delete;
    KVSTORE_API virtual ~KvStoreThreadPool() {};

    KVSTORE_API static std::shared_ptr<KvStoreThreadPool> GetPool(int poolSize, bool startImmediately = false);
    KVSTORE_API virtual void Stop() = 0;
    KVSTORE_API virtual bool AddTask(KvStoreTask &&task) = 0;
    KVSTORE_API static constexpr int MAX_POOL_SIZE = 64; // the max thread pool size
    KVSTORE_API static constexpr int DEFAULT_POOL_SIZE = 8; // the default thread pool size
protected:
    KvStoreThreadPool() = default;
};
} // namespace DistributedKv
} // namespace OHOS

#endif // KV_STORE_THREAD_POOL_H
