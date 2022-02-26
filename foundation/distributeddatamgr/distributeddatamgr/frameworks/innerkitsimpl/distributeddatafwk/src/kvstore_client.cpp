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

#define LOG_TAG "KvStoreClient"

#include "constant.h"
#include "dds_trace.h"
#include "kvstore_client.h"
#include "kvstore_observer_client.h"
#include "kvstore_snapshot_client.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
KvStoreClient::KvStoreClient(sptr<IKvStoreImpl> kvStoreProxy, const std::string &storeId)
    : kvStoreProxy_(std::move(kvStoreProxy)), storeId_(storeId)
{
    ZLOGI("construct");
}

KvStoreClient::~KvStoreClient()
{
    ZLOGI("destruct");
}

StoreId KvStoreClient::GetStoreId() const
{
    StoreId storeId;
    storeId.storeId = storeId_;
    return storeId;
}

void KvStoreClient::GetKvStoreSnapshot(std::shared_ptr<KvStoreObserver> observer,
                                       std::function<void(Status, std::unique_ptr<KvStoreSnapshot>)> callback) const
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    Status statusTmp = Status::SERVER_UNAVAILABLE;
    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        callback(statusTmp, nullptr);
        return;
    }

    sptr<KvStoreObserverClient> kvStoreObserverClient = new KvStoreObserverClient(GetStoreId(),
        SubscribeType::SUBSCRIBE_TYPE_ALL, observer, KvStoreType::MULTI_VERSION);

    sptr<IKvStoreSnapshotImpl> snapshotProxyTmp;
    auto snapshotCallbackFunction = [&](Status status, sptr<IKvStoreSnapshotImpl> snapshotProxy) {
        statusTmp = status;
        snapshotProxyTmp = snapshotProxy;
    };
    kvStoreProxy_->GetKvStoreSnapshot(kvStoreObserverClient, snapshotCallbackFunction);
    if (statusTmp != Status::SUCCESS) {
        ZLOGE("return error: %d.", static_cast<int>(statusTmp));
        callback(statusTmp, nullptr);
        return;
    }

    if (snapshotProxyTmp == nullptr) {
        ZLOGE("snapshotProxyTmp is nullptr.");
        callback(statusTmp, nullptr);
        return;
    }

    ZLOGD("success.");
    callback(statusTmp, std::make_unique<KvStoreSnapshotClient>(std::move(snapshotProxyTmp)));
}

Status KvStoreClient::ReleaseKvStoreSnapshot(std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    if (kvStoreProxy_ == nullptr) {
        ZLOGE("kvstore proxy is nullptr.");
        return Status::SERVER_UNAVAILABLE;
    }
    if (kvStoreSnapshotPtr == nullptr) {
        ZLOGE("kvstoresnapshot is nullptr.");
        return Status::INVALID_ARGUMENT;
    }

    KvStoreSnapshotClient *kvStoreSnapshotClient =
        reinterpret_cast<KvStoreSnapshotClient *>(kvStoreSnapshotPtr.release());
    sptr<IKvStoreSnapshotImpl> snapshotProxyTmp = kvStoreSnapshotClient->GetkvStoreSnapshotProxy();
    Status status = kvStoreProxy_->ReleaseKvStoreSnapshot(std::move(snapshotProxyTmp));
    delete kvStoreSnapshotClient;
    kvStoreSnapshotClient = nullptr;
    ZLOGI("return: %d.", static_cast<int>(status));
    return status;
}

Status KvStoreClient::Put(const Key &key, const Value &value)
{
    ZLOGD("key: %zu value: %zu.", key.Size(), value.Size());
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    if (keyData.size() == 0 || keyData.size() > Constant::MAX_KEY_LENGTH ||
        value.Size() > Constant::MAX_VALUE_LENGTH) {
        ZLOGE("invalid key or value.");
        return Status::INVALID_ARGUMENT;
    }

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->Put(key, value);
    }
    ZLOGE("kvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status KvStoreClient::PutBatch(const std::vector<Entry> &entries)
{
    ZLOGI("entry size: %zu", entries.size());
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (entries.size() > Constant::MAX_BATCH_SIZE) {
        ZLOGE("batch size must less than 128.");
        return Status::INVALID_ARGUMENT;
    }
    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->PutBatch(entries);
    }
    ZLOGE("kvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status KvStoreClient::Delete(const Key &key)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    if (keyData.size() == 0 || keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid key.");
        return Status::INVALID_ARGUMENT;
    }

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->Delete(key);
    }
    ZLOGE("kvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status KvStoreClient::DeleteBatch(const std::vector<Key> &keys)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (keys.size() > Constant::MAX_BATCH_SIZE) {
        ZLOGE("batch size must less than 128.");
        return Status::INVALID_ARGUMENT;
    }

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->DeleteBatch(keys);
    }
    ZLOGE("kvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status KvStoreClient::Clear()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->Clear();
    }
    ZLOGE("kvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status KvStoreClient::StartTransaction()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->StartTransaction();
    }
    ZLOGE("kvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status KvStoreClient::Commit()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->Commit();
    }
    ZLOGE("kvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status KvStoreClient::Rollback()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    if (kvStoreProxy_ != nullptr) {
        return kvStoreProxy_->Rollback();
    }
    ZLOGE("kvstore proxy is nullptr.");
    return Status::SERVER_UNAVAILABLE;
}

Status KvStoreClient::SubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer)
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
            new (std::nothrow) KvStoreObserverClient(GetStoreId(), subscribeType, observer, KvStoreType::MULTI_VERSION);
    if (ipcObserver == nullptr) {
        ZLOGW("new KvStoreObserverClient failed");
        return Status::ERROR;
    }
    Status status = kvStoreProxy_->SubscribeKvStore(subscribeType, ipcObserver);
    if (status == Status::SUCCESS) {
        registeredObservers_.emplace(observer.get(), ipcObserver);
    }
    return status;
}

Status KvStoreClient::UnSubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    if (observer == nullptr) {
        ZLOGW("return INVALID_ARGUMENT.");
        return Status::INVALID_ARGUMENT;
    }
    std::lock_guard<std::mutex> lck(observerMapMutex_);
    auto it = registeredObservers_.find(observer.get());
    if (it == registeredObservers_.end()) {
        ZLOGW("return STORE_NOT_SUBSCRIBE.");
        return Status::STORE_NOT_SUBSCRIBE;
    }
    Status status = kvStoreProxy_->UnSubscribeKvStore(subscribeType, it->second);
    if (status == Status::SUCCESS) {
        registeredObservers_.erase(it);
    }
    return status;
}
}  // namespace DistributedKv
}  // namespace OHOS
