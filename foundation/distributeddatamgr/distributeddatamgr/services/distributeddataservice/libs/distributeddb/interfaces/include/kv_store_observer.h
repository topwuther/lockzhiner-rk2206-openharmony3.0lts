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

#ifndef KV_STORE_OBSERVER_H
#define KV_STORE_OBSERVER_H

#include "kv_store_changed_data.h"

namespace DistributedDB {
class KvStoreObserver {
public:
    virtual ~KvStoreObserver() {}

    // Databa change callback
    virtual void OnChange(const KvStoreChangedData &data) = 0;
};
} // namespace DistributedDB

#endif // KV_STORE_OBSERVER_H