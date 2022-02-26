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

#define LOG_TAG "SingleKvStoreClient"

#include "single_kvstore_client.h"
#include "constant.h"
#include "dds_trace.h"
#include "kvstore_observer_client.h"
#include "kvstore_resultset_client.h"
#include "kvstore_sync_callback_client.h"
#include "log_print.h"

namespace OHOS::DistributedKv {
SingleKvStoreClient::SingleKvStoreClient(sptr<ISingleKvStore> kvStoreProxy, const std::string &storeId)
    : kvStoreProxy_(kvStoreProxy), storeId_(storeId)
{}

StoreId SingleKvStoreClient::GetStoreId() const
{
    StoreId storeId;
    storeId.storeId = storeId_;
    return storeId;
}

Status SingleKvStoreClient::GetEntries(const Key &prefixKey, std::vector<Entry> &entries) const
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        return Status::SERVER_UNAVAILABLE;
    }

    return kvStoreProxy_->GetEntries(prefixKey, entries);
}

Status SingleKvStoreClient::GetEntriesWithQuery(const std::string &query, std::vector<Entry> &entries) const
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        return Status::SERVER_UNAVAILABLE;
    }
    ZLOGD("Cpp client GetEntriesWithQuery");
    return kvStoreProxy_->GetEntriesWithQuery(query, entries);
}

Status SingleKvStoreClient::GetEntriesWithQuery(const DataQuery &query, std::vector<Entry> &entries) const
{
    return GetEntriesWithQuery(query.ToString(), entries);
}

void SingleKvStoreClient::GetResultSet(const Key &prefixKey,
    std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    Status statusTmp = Status::SERVER_UNAVAILABLE;
    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        callback(statusTmp, nullptr);
        return;
    }
    sptr<IKvStoreResultSet> resultSetTmp;
    auto callFun = [&](Status status, sptr<IKvStoreResultSet> proxy) {
        statusTmp = status;
        resultSetTmp = proxy;
    };
    kvStoreProxy_->GetResultSet(prefixKey, callFun);
    if (statusTmp != Status::SUCCESS) {
        ZLOGE("return error: %d.", static_cast<int>(statusTmp));
        callback(statusTmp, nullptr);
        return;
    }

    if (resultSetTmp == nullptr) {
        ZLOGE("resultSetTmp is nullptr.");
        callback(statusTmp, nullptr);
        return;
    }
    callback(statusTmp, std::make_unique<KvStoreResultSetClient>(std::move(resultSetTmp)));
}

void SingleKvStoreClient::GetResultSetWithQuery(const std::string &query,
    std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    Status statusTmp = Status::SERVER_UNAVAILABLE;
    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        callback(statusTmp, nullptr);
        return;
    }

    ZLOGD("Cpp client GetResultSetWithQuery");
    sptr<IKvStoreResultSet> resultSetTmp;
    auto callFun = [&](Status status, sptr<IKvStoreResultSet> proxy) {
        statusTmp = status;
        resultSetTmp = proxy;
    };
    kvStoreProxy_->GetResultSetWithQuery(query, callFun);
    if (statusTmp != Status::SUCCESS) {
        ZLOGE("return error: %d.", static_cast<int>(statusTmp));
        callback(statusTmp, nullptr);
        return;
    }

    if (resultSetTmp == nullptr) {
        ZLOGE("resultSetTmp is nullptr.");
        callback(statusTmp, nullptr);
        return;
    }
    callback(statusTmp, std::make_unique<KvStoreResultSetClient>(std::move(resultSetTmp)));
    ZLOGE("GetResultSetWithQuery");
}

void SingleKvStoreClient::GetResultSetWithQuery(const DataQuery &query,
    std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const
{
    GetResultSetWithQuery(query.ToString(), callback);
}

Status SingleKvStoreClient::CloseResultSet(std::unique_ptr<KvStoreResultSet> resultSet)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    if (resultSet == nullptr) {
        ZLOGE("resultSet is nullptr.");
        return Status::INVALID_ARGUMENT;
    }
    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        return Status::SERVER_UNAVAILABLE;
    }
    auto resultSetClient = reinterpret_cast<KvStoreResultSetClient *>(resultSet.release());
    return kvStoreProxy_->CloseResultSet(resultSetClient->GetKvStoreResultSetProxy());
}

Status SingleKvStoreClient::GetCountWithQuery(const std::string &query, int &result) const
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        return Status::SERVER_UNAVAILABLE;
    }
    ZLOGD("Cpp client GetCountWithQuery");
    return kvStoreProxy_->GetCountWithQuery(query, result);
}

Status SingleKvStoreClient::GetCountWithQuery(const DataQuery &query, int &result) const
{
    return GetCountWithQuery(query.ToString(), result);
}

Status SingleKvStoreClient::Sync(const std::vector<std::string> &deviceIdList, const SyncMode &mode,
    uint32_t allowedDelayMs)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        return Status::SERVER_UNAVAILABLE;
    }
    if (deviceIdList.empty()) {
        ZLOGW("deviceIdList is empty.");
        return Status::INVALID_ARGUMENT;
    }
    return kvStoreProxy_->Sync(deviceIdList, mode, allowedDelayMs);
}

Status SingleKvStoreClient::RemoveDeviceData(const std::string &device)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        return Status::SERVER_UNAVAILABLE;
    }
    if (device.empty()) {
        ZLOGW("device is empty.");
        return Status::INVALID_ARGUMENT;
    }
    return kvStoreProxy_->RemoveDeviceData(device);
}

Status SingleKvStoreClient::Delete(const Key &key)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    ZLOGI("begin.");
    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    if (keyData.size() == 0 || keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid key.");
        return Status::INVALID_ARGUMENT;
    }

    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        return Status::SERVER_UNAVAILABLE;
    }
    return kvStoreProxy_->Delete(key);
}

Status SingleKvStoreClient::Put(const Key &key, const Value &value)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    ZLOGI("key: %zu value: %zu.", key.Size(), value.Size());
    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    if (keyData.size() == 0 || keyData.size() > Constant::MAX_KEY_LENGTH ||
        value.Size() > Constant::MAX_VALUE_LENGTH) {
        ZLOGE("invalid key or value.");
        return Status::INVALID_ARGUMENT;
    }
    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        return Status::SERVER_UNAVAILABLE;
    }
    return kvStoreProxy_->Put(key, value);
}

Status SingleKvStoreClient::Get(const Key &key, Value &value)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        return Status::SERVER_UNAVAILABLE;
    }
    return kvStoreProxy_->Get(key, value);
}

Status SingleKvStoreClient::SubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    if (observer == nullptr) {
        ZLOGW("return INVALID_ARGUMENT.");
        return Status::INVALID_ARGUMENT;
    }
    std::lock_guard<std::mutex> lck(observerMapMutex_);
    // change this to map.contains() after c++20
    if (registeredObservers_.count(observer.get()) == 1) {
        ZLOGW("return STORE_ALREADY_SUBSCRIBE.");
        return Status::STORE_ALREADY_SUBSCRIBE;
    }
    // remove storeId after remove SubscribeKvStore function in manager. currently reserve for convenience.
    sptr<KvStoreObserverClient> ipcObserver =
            new (std::nothrow) KvStoreObserverClient(GetStoreId(), subscribeType, observer,
                    KvStoreType::SINGLE_VERSION);
    if (ipcObserver == nullptr) {
        ZLOGW("new KvStoreObserverClient failed");
        return Status::ERROR;
    }
    Status status = kvStoreProxy_->SubscribeKvStore(subscribeType, ipcObserver);
    if (status == Status::SUCCESS) {
        const auto temp = registeredObservers_.insert({observer.get(), ipcObserver});
        if (!temp.second) {
            ZLOGW("local insert error");
            return Status::ERROR;
        }
    }
    return status;
}

Status SingleKvStoreClient::UnSubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    if (observer == nullptr) {
        ZLOGW("return INVALID_ARGUMENT.");
        return Status::INVALID_ARGUMENT;
    }
    std::lock_guard<std::mutex> lck(observerMapMutex_);
    auto it = registeredObservers_.find(observer.get());
    if (it == registeredObservers_.end()) {
        ZLOGW(" STORE NOT SUBSCRIBE.");
        return Status::STORE_NOT_SUBSCRIBE;
    }
    Status status = kvStoreProxy_->UnSubscribeKvStore(subscribeType, it->second);
    if (status == Status::SUCCESS) {
        registeredObservers_.erase(it);
    } else {
        ZLOGW("single unSubscribe failed code=%d.", static_cast<int>(status));
    }
    return status;
}

Status SingleKvStoreClient::RegisterSyncCallback(std::shared_ptr<KvStoreSyncCallback> callback)
{
    ZLOGI("begin.");
    if (callback == nullptr) {
        ZLOGW("return INVALID_ARGUMENT.");
        return Status::INVALID_ARGUMENT;
    }
    // remove storeId after remove SubscribeKvStore function in manager. currently reserve for convenience.
    sptr<KvStoreSyncCallbackClient> ipcCallback =
            new (std::nothrow) KvStoreSyncCallbackClient(callback);
    if (ipcCallback == nullptr) {
        ZLOGW("new KvStoreSyncCallbackClient failed");
        return Status::ERROR;
    }
    return kvStoreProxy_->RegisterSyncCallback(ipcCallback);
}

Status SingleKvStoreClient::UnRegisterSyncCallback()
{
    ZLOGI("begin.");
    return kvStoreProxy_->UnRegisterSyncCallback();
}

Status SingleKvStoreClient::PutBatch(const std::vector<Entry> &entries)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    ZLOGI("entry size: %zu", entries.size());
    if (entries.size() > Constant::MAX_BATCH_SIZE) {
        ZLOGE("batch size must less than 128.");
        return Status::INVALID_ARGUMENT;
    }
    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->PutBatch(entries);
    }
    ZLOGE("singleKvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status SingleKvStoreClient::DeleteBatch(const std::vector<Key> &keys)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    if (keys.size() > Constant::MAX_BATCH_SIZE) {
        ZLOGE("batch size must less than 128.");
        return Status::INVALID_ARGUMENT;
    }

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->DeleteBatch(keys);
    }
    ZLOGE("singleKvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status SingleKvStoreClient::StartTransaction()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->StartTransaction();
    }
    ZLOGE("singleKvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status SingleKvStoreClient::Commit()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->Commit();
    }
    ZLOGE("singleKvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status SingleKvStoreClient::Rollback()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->Rollback();
    }
    ZLOGE("singleKvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status SingleKvStoreClient::SetSyncParam(const KvSyncParam &syncParam)
{
    KvParam input(TransferTypeToByteArray<KvSyncParam>(syncParam));
    sptr<KvParam> output = nullptr;
    return Control(KvControlCmd::SET_SYNC_PARAM, input, output);
}

Status SingleKvStoreClient::GetSyncParam(KvSyncParam &syncParam)
{
    KvParam inputEmpty;
    sptr<KvParam> output = nullptr;
    Status ret = Control(KvControlCmd::GET_SYNC_PARAM, inputEmpty, output);
    if (ret != Status::SUCCESS) {
        return ret;
    }
    if ((output != nullptr) && (output->Size() == sizeof(syncParam))) {
        syncParam = TransferByteArrayToType<KvSyncParam>(output->Data());
        return Status::SUCCESS;
    }
    return Status::ERROR;
}

Status SingleKvStoreClient::Control(KvControlCmd cmd, const KvParam &inputParam, sptr<KvParam> &output)
{
    ZLOGI("begin.");
    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->Control(cmd, inputParam, output);
    }
    ZLOGE("singleKvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}
Status SingleKvStoreClient::SetCapabilityEnabled(bool enabled) const
{
    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->SetCapabilityEnabled(enabled);
    }
    ZLOGE("singleKvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status SingleKvStoreClient::SetCapabilityRange(const std::vector<std::string> &localLabels,
                                               const std::vector<std::string> &remoteSupportLabels) const
{
    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->SetCapabilityRange(localLabels, remoteSupportLabels);
    }
    ZLOGE("singleKvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status SingleKvStoreClient::GetSecurityLevel(SecurityLevel &securityLevel) const
{
    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->GetSecurityLevel(securityLevel);
    }
    ZLOGE("singleKvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}
} // namespace OHOS::DistributedKv
