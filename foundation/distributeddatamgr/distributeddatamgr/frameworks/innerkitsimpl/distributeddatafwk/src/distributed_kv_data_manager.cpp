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

#define LOG_TAG "DistributedKvDataManager"

#include "distributed_kv_data_manager.h"
#include "constant.h"
#include "ikvstore_data_service.h"
#include "kvstore_client.h"
#include "kvstore_service_death_notifier.h"
#include "log_print.h"
#include "refbase.h"
#include "single_kvstore_client.h"
#include "dds_trace.h"
#include "communication_provider.h"
#include "device_status_change_listener_client.h"

namespace OHOS {
namespace DistributedKv {
DistributedKvDataManager::DistributedKvDataManager()
{}

DistributedKvDataManager::~DistributedKvDataManager()
{}

void DistributedKvDataManager::GetKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
                                          std::function<void(Status, std::unique_ptr<KvStore>)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    std::string storeIdTmp = Constant::TrimCopy<std::string>(storeId.storeId);
    if (storeIdTmp.size() == 0 || storeIdTmp.size() > Constant::MAX_STORE_ID_LENGTH) {
        callback(Status::INVALID_ARGUMENT, nullptr);
        ZLOGE("invalid storeId.");
        return;
    }

    KvStoreServiceDeathNotifier::SetAppId(appId);
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    Status status = Status::SERVER_UNAVAILABLE;
    if (kvDataServiceProxy == nullptr) {
        ZLOGE("proxy is nullptr.");
        callback(status, nullptr);
        return;
    }

    ZLOGD("call proxy.");
    sptr<IKvStoreImpl> proxyTmp;
    status = kvDataServiceProxy->GetKvStore(options, appId, storeId,
        [&](sptr<IKvStoreImpl> proxy) { proxyTmp = std::move(proxy); });
    if (status == Status::RECOVER_SUCCESS) {
        ZLOGE("proxy recover success: %d", static_cast<int>(status));
        callback(status, std::make_unique<KvStoreClient>(std::move(proxyTmp), storeIdTmp));
        return;
    }

    if (status != Status::SUCCESS) {
        ZLOGE("proxy return error: %d", static_cast<int>(status));
        callback(status, nullptr);
        return;
    }

    if (proxyTmp == nullptr) {
        ZLOGE("proxy return nullptr.");
        callback(status, nullptr);
        return;
    }

    callback(status, std::make_unique<KvStoreClient>(std::move(proxyTmp), storeIdTmp));
}

void DistributedKvDataManager::GetSingleKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
    std::function<void(Status, std::unique_ptr<SingleKvStore>)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    std::string storeIdTmp = Constant::TrimCopy<std::string>(storeId.storeId);
    if (storeIdTmp.size() == 0 || storeIdTmp.size() > Constant::MAX_STORE_ID_LENGTH) {
        callback(Status::INVALID_ARGUMENT, nullptr);
        ZLOGE("invalid storeId.");
        return;
    }

    KvStoreServiceDeathNotifier::SetAppId(appId);
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    Status status = Status::SERVER_UNAVAILABLE;
    if (kvDataServiceProxy == nullptr) {
        ZLOGE("proxy is nullptr.");
        callback(status, nullptr);
        return;
    }

    ZLOGD("call proxy.");
    sptr<ISingleKvStore> proxyTmp;
    status = kvDataServiceProxy->GetSingleKvStore(options, appId, storeId,
        [&](sptr<ISingleKvStore> proxy) { proxyTmp = std::move(proxy); });
    if (status == Status::RECOVER_SUCCESS) {
        ZLOGE("proxy recover success: %d", static_cast<int>(status));
        callback(status, std::make_unique<SingleKvStoreClient>(std::move(proxyTmp), storeIdTmp));
        return;
    }

    if (status != Status::SUCCESS) {
        ZLOGE("proxy return error: %d", static_cast<int>(status));
        callback(status, nullptr);
        return;
    }

    if (proxyTmp == nullptr) {
        ZLOGE("proxy return nullptr.");
        callback(status, nullptr);
        return;
    }

    callback(status, std::make_unique<SingleKvStoreClient>(std::move(proxyTmp), storeIdTmp));
}

void DistributedKvDataManager::GetAllKvStoreId(const AppId &appId,
                                               std::function<void(Status, std::vector<StoreId> &)> callback)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KvStoreServiceDeathNotifier::SetAppId(appId);
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    if (kvDataServiceProxy == nullptr) {
        ZLOGE("proxy is nullptr.");
        std::vector<StoreId> storeIds;
        callback(Status::SERVER_UNAVAILABLE, storeIds);
        return;
    }

    kvDataServiceProxy->GetAllKvStoreId(appId, callback);
}

Status DistributedKvDataManager::CloseKvStore(const AppId &appId, const StoreId &storeId,
                                              std::unique_ptr<KvStore> kvStorePtr)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KvStoreServiceDeathNotifier::SetAppId(appId);
    std::string storeIdTmp = Constant::TrimCopy<std::string>(storeId.storeId);
    if (storeIdTmp.size() == 0 || storeIdTmp.size() > Constant::MAX_STORE_ID_LENGTH) {
        ZLOGE("invalid storeId.");
        return Status::INVALID_ARGUMENT;
    }

    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    if (kvDataServiceProxy != nullptr) {
        return kvDataServiceProxy->CloseKvStore(appId, storeId);
    }
    ZLOGE("proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status DistributedKvDataManager::CloseKvStore(const AppId &appId, std::unique_ptr<SingleKvStore> kvStorePtr)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    if (kvStorePtr == nullptr) {
        ZLOGE("kvStorePtr is nullptr.");
        return Status::INVALID_ARGUMENT;
    }
    KvStoreServiceDeathNotifier::SetAppId(appId);
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    if (kvDataServiceProxy != nullptr) {
        return kvDataServiceProxy->CloseKvStore(appId, kvStorePtr->GetStoreId());
    }
    ZLOGE("proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status DistributedKvDataManager::CloseAllKvStore(const AppId &appId)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KvStoreServiceDeathNotifier::SetAppId(appId);
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    if (kvDataServiceProxy != nullptr) {
        return kvDataServiceProxy->CloseAllKvStore(appId);
    }
    ZLOGE("proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status DistributedKvDataManager::DeleteKvStore(const AppId &appId, const StoreId &storeId)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::string storeIdTmp = Constant::TrimCopy<std::string>(storeId.storeId);
    if (storeIdTmp.size() == 0 || storeIdTmp.size() > Constant::MAX_STORE_ID_LENGTH) {
        ZLOGE("invalid storeId.");
        return Status::INVALID_ARGUMENT;
    }

    KvStoreServiceDeathNotifier::SetAppId(appId);
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    if (kvDataServiceProxy != nullptr) {
        return kvDataServiceProxy->DeleteKvStore(appId, storeId);
    }
    ZLOGE("proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status DistributedKvDataManager::DeleteAllKvStore(const AppId &appId)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KvStoreServiceDeathNotifier::SetAppId(appId);
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    if (kvDataServiceProxy != nullptr) {
        return kvDataServiceProxy->DeleteAllKvStore(appId);
    }
    ZLOGE("proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

void DistributedKvDataManager::RegisterKvStoreServiceDeathRecipient(
    std::shared_ptr<KvStoreDeathRecipient> kvStoreDeathRecipient)
{
    ZLOGD("begin");
    if (kvStoreDeathRecipient == nullptr) {
        ZLOGW("Register KvStoreService Death Recipient input is null.");
        return;
    }
    std::shared_ptr<KvStoreDeathRecipientImpl> kvStoreDeathRecipientImpl =
        std::make_shared<KvStoreDeathRecipientImpl>(kvStoreDeathRecipient);
    if (kvStoreDeathRecipientImpl != nullptr) {
        KvStoreServiceDeathNotifier::AddServiceDeathWatcher(kvStoreDeathRecipientImpl);
    } else {
        ZLOGW("Register KvStoreService Death Recipient failed.");
    }
}

void DistributedKvDataManager::UnRegisterKvStoreServiceDeathRecipient(
    std::shared_ptr<KvStoreDeathRecipient> kvStoreDeathRecipient)
{
    ZLOGD("begin");
    if (kvStoreDeathRecipient == nullptr) {
        ZLOGW("UnRegister KvStoreService Death Recipient input is null.");
        return;
    }
    std::shared_ptr<KvStoreDeathRecipientImpl> kvStoreDeathRecipientImpl =
        std::make_shared<KvStoreDeathRecipientImpl>(kvStoreDeathRecipient);
    if (kvStoreDeathRecipientImpl != nullptr) {
        KvStoreServiceDeathNotifier::RemoveServiceDeathWatcher(kvStoreDeathRecipientImpl);
    } else {
        ZLOGW("UnRegister KvStoreService Death Recipient failed.");
    }
}

Status DistributedKvDataManager::GetLocalDevice(DeviceInfo &localDevice)
{
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    if (kvDataServiceProxy == nullptr) {
        ZLOGE("proxy is nullptr.");
        return Status::ERROR;
    }

    return kvDataServiceProxy->GetLocalDevice(localDevice);
}

Status DistributedKvDataManager::GetDeviceList(std::vector<DeviceInfo> &deviceInfoList, DeviceFilterStrategy strategy)
{
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    if (kvDataServiceProxy == nullptr) {
        ZLOGE("proxy is nullptr.");
        return Status::ERROR;
    }

    return kvDataServiceProxy->GetDeviceList(deviceInfoList, strategy);
}

static std::map<DeviceStatusChangeListener *, sptr<IDeviceStatusChangeListener>> deviceObservers_;
static std::mutex deviceObserversMapMutex_;
Status DistributedKvDataManager::StartWatchDeviceChange(std::shared_ptr<DeviceStatusChangeListener> observer)
{
    sptr<DeviceStatusChangeListenerClient> ipcObserver = new(std::nothrow) DeviceStatusChangeListenerClient(observer);
    if (ipcObserver == nullptr) {
        ZLOGW("new DeviceStatusChangeListenerClient failed");
        return Status::ERROR;
    }
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    if (kvDataServiceProxy == nullptr) {
        ZLOGE("proxy is nullptr.");
        return Status::ERROR;
    }
    Status status = kvDataServiceProxy->StartWatchDeviceChange(ipcObserver, observer->GetFilterStrategy());
    if (status == Status::SUCCESS) {
        {
            std::lock_guard<std::mutex> lck(deviceObserversMapMutex_);
            deviceObservers_.insert({observer.get(), ipcObserver});
        }
        return Status::SUCCESS;
    }
    ZLOGE("watch failed.");
    return Status::ERROR;
}

Status DistributedKvDataManager::StopWatchDeviceChange(std::shared_ptr<DeviceStatusChangeListener> observer)
{
    sptr<IKvStoreDataService> kvDataServiceProxy = KvStoreServiceDeathNotifier::GetDistributedKvDataService();
    if (kvDataServiceProxy == nullptr) {
        ZLOGE("proxy is nullptr.");
        return Status::ERROR;
    }
    std::lock_guard<std::mutex> lck(deviceObserversMapMutex_);
    auto it = deviceObservers_.find(observer.get());
    if (it == deviceObservers_.end()) {
        ZLOGW(" not start watch device change.");
        return Status::ERROR;
    }
    Status status = kvDataServiceProxy->StopWatchDeviceChange(it->second);
    if (status == Status::SUCCESS) {
        deviceObservers_.erase(it->first);
    } else {
        ZLOGW("stop watch failed code=%d.", static_cast<int>(status));
    }
    return status;
}
}  // namespace DistributedKv
}  // namespace OHOS
