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

#define LOG_TAG "KvStoreObserverClient"

#include "kvstore_observer_client.h"
#include "kvstore_snapshot_client.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
KvStoreObserverClient::KvStoreObserverClient(const StoreId &storeId, SubscribeType subscribeType,
                                             std::shared_ptr<KvStoreObserver> kvStoreObserver, KvStoreType type)
    : storeId_(storeId), subscribeType_(subscribeType), kvStoreObserver_(kvStoreObserver), type_(type)
{
    ZLOGI("start");
}

KvStoreObserverClient::~KvStoreObserverClient()
{
    ZLOGI("start");
}

void KvStoreObserverClient::OnChange(const ChangeNotification &changeNotification, sptr<IKvStoreSnapshotImpl> snapshot)
{
    ZLOGI("start");
    if (kvStoreObserver_ != nullptr) {
        if (type_ == KvStoreType::SINGLE_VERSION) {
            ZLOGI("SINGLE_VERSION start");
            kvStoreObserver_->OnChange(changeNotification);
        } else {
            ZLOGI("MULTI_VERSION start");
            kvStoreObserver_->OnChange(changeNotification,
                                       std::make_unique<KvStoreSnapshotClient>(std::move(snapshot)));
        }
    }
}

const StoreId &KvStoreObserverClient::GetStoreId() const
{
    return storeId_;
}

const SubscribeType &KvStoreObserverClient::GetSubscribeType() const
{
    return subscribeType_;
}

const std::shared_ptr<KvStoreObserver> KvStoreObserverClient::GetKvStoreObserver() const
{
    return kvStoreObserver_;
}
}  // namespace DistributedKv
}  // namespace OHOS
