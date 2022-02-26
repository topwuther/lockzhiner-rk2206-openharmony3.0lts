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

#define LOG_TAG "KvStoreObserverImpl"

#include "kvstore_observer_impl.h"
#include <chrono>
#include <ctime>
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
using namespace std::chrono;

KvStoreObserverImpl::KvStoreObserverImpl(SubscribeType subscribeType, sptr<IKvStoreObserver> observerProxy)
    : subscribeType_(subscribeType), observerProxy_(std::move(observerProxy))
{
    ZLOGI("construct");
}

KvStoreObserverImpl::~KvStoreObserverImpl()
{
    ZLOGI("destruct");
}

// Data change callback
void KvStoreObserverImpl::OnChange(const DistributedDB::KvStoreChangedData &data)
{
    ZLOGI("onchange");
    if (observerProxy_ == nullptr) {
        ZLOGE("observerProxy_ is nullptr.");
        return;
    }
    std::list<DistributedDB::Entry> insertList = data.GetEntriesInserted();
    std::list<DistributedDB::Entry> updateList = data.GetEntriesUpdated();
    std::list<DistributedDB::Entry> deletedList = data.GetEntriesDeleted();

    std::list<Entry> insertListTmp;
    std::list<Entry> updateListTmp;
    std::list<Entry> deletedListTmp;

    for (const auto &entry : insertList) {
        Key key(entry.key);
        Value value(entry.value);
        Entry tmpEntry;
        tmpEntry.key = key;
        tmpEntry.value = value;
        insertListTmp.push_back(tmpEntry);
    }

    for (const auto &entry : updateList) {
        Key key(entry.key);
        Value value(entry.value);
        Entry tmpEntry;
        tmpEntry.key = key;
        tmpEntry.value = value;
        updateListTmp.push_back(tmpEntry);
    }

    for (const auto &entry : deletedList) {
        Key key(entry.key);
        Value value(entry.value);
        Entry tmpEntry;
        tmpEntry.key = key;
        tmpEntry.value = value;
        deletedListTmp.push_back(tmpEntry);
    }

    ChangeNotification changeNotification(insertListTmp, updateListTmp, deletedListTmp, std::string(), false);
    ZLOGI("call proxy OnChange");
    observerProxy_->OnChange(changeNotification, nullptr);
}

SubscribeType KvStoreObserverImpl::GetSubscribeType() const
{
    return subscribeType_;
}

sptr<IKvStoreObserver> KvStoreObserverImpl::GetKvStoreObserverProxy() const
{
    return observerProxy_;
}
}  // namespace DistributedKv
}  // namespace OHOS
