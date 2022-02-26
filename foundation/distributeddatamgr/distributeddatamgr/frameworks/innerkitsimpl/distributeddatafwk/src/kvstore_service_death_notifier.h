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

#ifndef KVSTORE_SERVICE_DEATH_NOTIFIER_H
#define KVSTORE_SERVICE_DEATH_NOTIFIER_H

#include <memory>
#include <set>
#include <thread>
#include "ikvstore_data_service.h"
#include "iremote_object.h"
#include "kvstore_death_recipient_impl.h"
#include "refbase.h"

namespace OHOS {
namespace DistributedKv {
struct KvStoreDeathRecipientImplCompare {
    bool operator()(const std::shared_ptr<KvStoreDeathRecipientImpl> lhs,
                    const std::shared_ptr<KvStoreDeathRecipientImpl> rhs) const
    {
        return lhs->kvStoreDeathRecipient_ < rhs->kvStoreDeathRecipient_;
    }
};

class KvStoreServiceDeathNotifier final {
public:
    KvStoreServiceDeathNotifier() = delete;
    ~KvStoreServiceDeathNotifier() = delete;
    // get DistributedKvDataService proxy object.
    static sptr<IKvStoreDataService> GetDistributedKvDataService();
    // temporarily used, should get in service side from binder.
    static void SetAppId(const AppId &appId);
    // add watcher for server die msg.
    static void AddServiceDeathWatcher(std::shared_ptr<KvStoreDeathRecipientImpl> watcher);
    // remove watcher for server die msg.
    static void RemoveServiceDeathWatcher(std::shared_ptr<KvStoreDeathRecipientImpl> watcher);
private:
    class KvStoreDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        KvStoreDeathRecipient();

        virtual ~KvStoreDeathRecipient();

        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };

    // add watcher for server die msg.
    static void RegisterClientDeathObserver();
    static AppId appId_;
    // lock for kvDataServiceProxy_ and serviceDeathWatchers_.
    static std::mutex watchMutex_;
    static sptr<IKvStoreDataService> kvDataServiceProxy_;
    static sptr<KvStoreDeathRecipient> deathRecipientPtr_;
    static sptr<IRemoteObject> clientDeathObserverPtr_;
    // set of watchers for server die msg.
    static std::set<std::shared_ptr<KvStoreDeathRecipientImpl>, KvStoreDeathRecipientImplCompare> serviceDeathWatchers_;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // KVSTORE_SERVICE_DEATH_NOTIFIER_H
