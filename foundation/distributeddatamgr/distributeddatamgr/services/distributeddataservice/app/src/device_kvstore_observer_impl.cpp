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

#define LOG_TAG "AppDistributedKvDataManagerImpl"

#include "device_kvstore_observer_impl.h"
#include "kvstore_utils.h"
#include "log_print.h"
using namespace OHOS::AppDistributedKv;
namespace OHOS::DistributedKv {
DeviceKvStoreObserverImpl::DeviceKvStoreObserverImpl(SubscribeType subscribeType, sptr<IKvStoreObserver> observerProxy,
    bool deviceSync) : KvStoreObserverImpl(subscribeType, observerProxy), deviceSync_(deviceSync),
    localDeviceId_{}, observerProxy_(observerProxy)
{}

DeviceKvStoreObserverImpl::~DeviceKvStoreObserverImpl()
{}

void DeviceKvStoreObserverImpl::OnChange(const DistributedDB::KvStoreChangedData &data)
{
    if (deviceSync_) {
        std::list<DistributedDB::Entry> insertList = data.GetEntriesInserted();
        std::list<DistributedDB::Entry> updateList = data.GetEntriesUpdated();
        std::list<DistributedDB::Entry> deletedList = data.GetEntriesDeleted();

        std::list<Entry> insertListTmp;
        std::list<Entry> updateListTmp;
        std::list<Entry> deletedListTmp;
        std::string deviceId;
        Transfer(insertList, insertListTmp, deviceId);
        Transfer(updateList, updateListTmp, deviceId);
        Transfer(deletedList, deletedListTmp, deviceId);
        if (deviceId.empty()) {
            ZLOGE("Did NOT find any valid deviceId");
        }
        ChangeNotification changeNotification(insertListTmp, updateListTmp, deletedListTmp, deviceId, false);
        if (observerProxy_ != nullptr) {
            observerProxy_->OnChange(changeNotification, nullptr);
        }
        return;
    } else {
        KvStoreObserverImpl::OnChange(data);
    }
}

void DeviceKvStoreObserverImpl::Transfer(const std::list<DistributedDB::Entry> &input, std::list<Entry> &output,
                                         std::string &deviceId)
{
    if (localDeviceId_.empty()) {
        auto localDevId = KvStoreUtils::GetProviderInstance().GetLocalDevice().deviceId;
        if (localDevId.empty()) {
            return;
        }
        localDeviceId_ = localDevId;
    }

    for (const auto &entry : input) {
        if (localDeviceId_.size() > entry.key.size()) {
            continue;
        }
        if (deviceId.empty()) {
            ZLOGI("Get deviceId from Entry.");
            auto uuid = std::string(entry.key.begin(), entry.key.begin() + localDeviceId_.length());
            if (localDeviceId_.compare(uuid) == 0) {
                deviceId = KvStoreUtils::GetProviderInstance().GetLocalBasicInfo().deviceId;
            } else {
                deviceId = KvStoreUtils::GetProviderInstance().ToNodeId(uuid);
            }
        }
        std::vector<uint8_t> decorateKey(entry.key.begin() + localDeviceId_.length(), entry.key.end() - sizeof(int));
        Entry tmpEntry;
        tmpEntry.key = decorateKey;
        tmpEntry.value = entry.value;
        output.push_back(tmpEntry);
    }
}
}