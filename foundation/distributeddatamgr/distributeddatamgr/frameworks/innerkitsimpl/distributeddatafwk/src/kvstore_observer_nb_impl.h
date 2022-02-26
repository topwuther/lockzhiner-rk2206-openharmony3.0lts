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

#include <list>
#include "app_types.h"
#include "kv_store_observer.h"
#include "log_print.h"

namespace OHOS {
namespace AppDistributedKv {
class KvStoreObserverNbImpl : public DistributedDB::KvStoreObserver {
public:
    KvStoreObserverNbImpl(AppKvStoreObserver *appKvStoreObserver, const SubscribeType &subscribeType)
    {
        appKvStoreObserver_ = appKvStoreObserver;
        subscribeType_ = subscribeType;
    }

    virtual void OnChange(const DistributedDB::KvStoreChangedData &data)
    {
        if (appKvStoreObserver_ == nullptr) {
            ZLOGE("appKvStoreObserver_ is nullptr.");
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

        AppChangeNotification changeNotification(insertListTmp, updateListTmp, deletedListTmp, std::string(), false);
        appKvStoreObserver_->OnChange(changeNotification);
    }

    virtual ~KvStoreObserverNbImpl()
    {}
private:
    AppKvStoreObserver *appKvStoreObserver_;
    SubscribeType subscribeType_;
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif  // APP_KV_STORE_OBSERVER_H
