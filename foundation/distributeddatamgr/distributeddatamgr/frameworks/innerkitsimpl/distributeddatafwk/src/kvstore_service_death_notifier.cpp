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

#define LOG_TAG "KvStoreServiceDeathNotifier"

#include "kvstore_service_death_notifier.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "kvstore_client_death_observer.h"
#include "log_print.h"
#include "refbase.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedKv {
AppId KvStoreServiceDeathNotifier::appId_;
std::mutex KvStoreServiceDeathNotifier::watchMutex_;
sptr<IKvStoreDataService> KvStoreServiceDeathNotifier::kvDataServiceProxy_;
sptr<KvStoreServiceDeathNotifier::KvStoreDeathRecipient> KvStoreServiceDeathNotifier::deathRecipientPtr_;
sptr<IRemoteObject> KvStoreServiceDeathNotifier::clientDeathObserverPtr_;
std::set<std::shared_ptr<KvStoreDeathRecipientImpl>, KvStoreDeathRecipientImplCompare>
    KvStoreServiceDeathNotifier::serviceDeathWatchers_;

void KvStoreServiceDeathNotifier::SetAppId(const AppId &appId)
{
    appId_ = appId;
}

sptr<IKvStoreDataService> KvStoreServiceDeathNotifier::GetDistributedKvDataService()
{
    ZLOGD("begin.");
    std::lock_guard<std::mutex> lg(watchMutex_);
    if (kvDataServiceProxy_ != nullptr) {
        return kvDataServiceProxy_;
    }

    ZLOGI("create remote proxy.");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        ZLOGE("get samgr fail.");
        return nullptr;
    }

    auto remote = samgr->CheckSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
    kvDataServiceProxy_ = iface_cast<IKvStoreDataService>(remote);
    if (kvDataServiceProxy_ == nullptr) {
        ZLOGE("initialize proxy failed.");
        return nullptr;
    }

    if (deathRecipientPtr_ == nullptr) {
        deathRecipientPtr_ = new (std::nothrow) KvStoreDeathRecipient();
        if (deathRecipientPtr_ == nullptr) {
            ZLOGW("new KvStoreDeathRecipient failed");
            return nullptr;
        }
    }
    if ((remote->IsProxyObject()) && (!remote->AddDeathRecipient(deathRecipientPtr_))) {
        ZLOGE("failed to add death recipient.");
    }

    RegisterClientDeathObserver();

    return kvDataServiceProxy_;
}

void KvStoreServiceDeathNotifier::RegisterClientDeathObserver()
{
    if (kvDataServiceProxy_ == nullptr) {
        return;
    }
    if (clientDeathObserverPtr_ == nullptr) {
        clientDeathObserverPtr_ = new (std::nothrow) KvStoreClientDeathObserver();
    }
    if (clientDeathObserverPtr_ == nullptr) {
        ZLOGW("new KvStoreClientDeathObserver failed");
        return;
    }
    kvDataServiceProxy_->RegisterClientDeathObserver(appId_, clientDeathObserverPtr_);
}

void KvStoreServiceDeathNotifier::AddServiceDeathWatcher(std::shared_ptr<KvStoreDeathRecipientImpl> watcher)
{
    std::lock_guard<std::mutex> lg(watchMutex_);
    auto ret = serviceDeathWatchers_.insert(watcher);
    if (ret.second) {
        ZLOGI("success set size: %zu", serviceDeathWatchers_.size());
    } else {
        ZLOGE("failed set size: %zu", serviceDeathWatchers_.size());
    }
}

void KvStoreServiceDeathNotifier::RemoveServiceDeathWatcher(std::shared_ptr<KvStoreDeathRecipientImpl> watcher)
{
    std::lock_guard<std::mutex> lg(watchMutex_);
    auto it = serviceDeathWatchers_.find(watcher);
    if (it != serviceDeathWatchers_.end()) {
        serviceDeathWatchers_.erase(it);
        ZLOGI("find & erase set size: %zu", serviceDeathWatchers_.size());
    } else {
        ZLOGE("no found set size: %zu", serviceDeathWatchers_.size());
    }
}

void KvStoreServiceDeathNotifier::KvStoreDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    ZLOGW("DistributedDataMgrService died.");
    // Need to do this with the lock held
    std::lock_guard<std::mutex> lg(watchMutex_);
    kvDataServiceProxy_ = nullptr;
    ZLOGI("watcher set size: %zu", serviceDeathWatchers_.size());
    for (const auto &watcher : serviceDeathWatchers_) {
        std::thread th = std::thread([watcher]() {
            watcher->OnRemoteDied();
        });
        th.detach();
    }
}

KvStoreServiceDeathNotifier::KvStoreDeathRecipient::KvStoreDeathRecipient()
{
    ZLOGI("constructor.");
}

KvStoreServiceDeathNotifier::KvStoreDeathRecipient::~KvStoreDeathRecipient()
{
    ZLOGI("destructor.");
}
}  // namespace DistributedKv
}  // namespace OHOS
