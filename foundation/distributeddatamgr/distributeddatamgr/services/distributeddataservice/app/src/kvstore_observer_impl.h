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

#ifndef KVSTORE_OBSERVER_IMPL_H
#define KVSTORE_OBSERVER_IMPL_H

#include "ikvstore.h"
#include "ikvstore_observer.h"
#include "kv_store_delegate.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {

class KvStoreObserverImpl : public DistributedDB::KvStoreObserver {
public:
    KvStoreObserverImpl(SubscribeType subscribeType, sptr<IKvStoreObserver> observerProxy);

    virtual ~KvStoreObserverImpl();

    // Database change callback
    void OnChange(const DistributedDB::KvStoreChangedData &data) override;

    SubscribeType GetSubscribeType() const;
    sptr<IKvStoreObserver> GetKvStoreObserverProxy() const;

private:
    SubscribeType subscribeType_;
    sptr<IKvStoreObserver> observerProxy_;
};

}  // namespace DistributedKv
}  // namespace OHOS

#endif  // KVSTORE_OBSERVER_IMPL_H
