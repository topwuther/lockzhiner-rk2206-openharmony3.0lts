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

#define LOG_TAG "KvStoreImpl"

#include "kvstore_impl.h"
#include <chrono>
#include <thread>
#include <vector>
#include "backup_handler.h"
#include "constant.h"
#include "dds_trace.h"
#include "kvstore_account_observer.h"
#include "kvstore_data_service.h"
#include "kvstore_meta_manager.h"
#include "kvstore_utils.h"
#include "log_print.h"
#include "permission_validator.h"
#include "reporter.h"

namespace OHOS {
namespace DistributedKv {
KvStoreImpl::KvStoreImpl(const Options &options,
                         const std::string &deviceAccountId, const std::string &bundleName, const std::string &storeId,
                         const std::string &appDirectory, DistributedDB::KvStoreDelegate *kvStoreDelegate)
    : options_(options),
      deviceAccountId_(deviceAccountId),
      bundleName_(bundleName),
      storeId_(storeId),
      storePath_(Constant::Concatenate({ appDirectory, storeId })),
      kvStoreDelegate_(kvStoreDelegate),
      storeObserverMutex_(),
      observerSet_(),
      openCount_(1)
{
    ZLOGI("construct");
}

void KvStoreImpl::GetKvStoreSnapshot(sptr<IKvStoreObserver> observer,
                                     std::function<void(Status, sptr<IKvStoreSnapshotImpl>)> callback)
{
    ZLOGI("begin.");
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER();
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        callback(Status::DB_ERROR, nullptr);
        return;
    }

    DistributedDB::KvStoreSnapshotDelegate *retSnapshotKvStore = nullptr;
    DistributedDB::DBStatus retSnapshotStatus;
    auto snapshotCallbackFunction = [&](DistributedDB::DBStatus status,
                                        DistributedDB::KvStoreSnapshotDelegate *snapshot) {
        retSnapshotStatus = status;
        retSnapshotKvStore = snapshot;
    };
    std::lock_guard<std::mutex> lg(storeSnapshotMutex_);
    KvStoreObserverImpl *kvStoreObserverImpl = nullptr;
    if (observer == nullptr) {
        {
            DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
            kvStoreDelegate_->GetKvStoreSnapshot(nullptr, snapshotCallbackFunction);
        }
    } else {
        kvStoreObserverImpl =
                new (std::nothrow) KvStoreObserverImpl(SubscribeType::SUBSCRIBE_TYPE_ALL, observer);
        if (kvStoreObserverImpl == nullptr) {
            ZLOGW("new KvStoreObserverImpl failed");
            callback(Status::ERROR, nullptr);
            return;
        }
        {
            DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
            kvStoreDelegate_->GetKvStoreSnapshot(kvStoreObserverImpl, snapshotCallbackFunction);
        }
    }

    if (retSnapshotStatus != DistributedDB::DBStatus::OK || retSnapshotKvStore == nullptr) {
        ZLOGE("delegate return nullptr.");
        delete kvStoreObserverImpl;
        kvStoreObserverImpl = nullptr;
        if (retSnapshotStatus == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
            ZLOGE("GetKvStoreSnapshot failed, distributeddb need recover.");
            IMPORT_DATABASE(bundleName_);
        } else {
            callback(Status::DB_ERROR, nullptr);
        }
        return;
    }

    ZLOGD("get delegate");
    KvStoreSnapshotImpl *snapshot = new (std::nothrow) KvStoreSnapshotImpl(retSnapshotKvStore, kvStoreObserverImpl);
    if (snapshot == nullptr) {
        ZLOGW("new KvStoreSnapshotImpl failed");
        delete kvStoreObserverImpl;
        kvStoreObserverImpl = nullptr;
        callback(Status::ERROR, nullptr);
        {
            DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
            kvStoreDelegate_->ReleaseKvStoreSnapshot(retSnapshotKvStore);
        }
        return;
    }
    sptr<IKvStoreSnapshotImpl> kvStoreSnapshotImpl = snapshot;
    callback(Status::SUCCESS, kvStoreSnapshotImpl);
    snapshotMap_.emplace(snapshot, std::move(kvStoreSnapshotImpl));
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
}

Status KvStoreImpl::ReleaseKvStoreSnapshot(sptr<IKvStoreSnapshotImpl> iKvStoreSnapshot)
{
    ZLOGI("begin.");
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is nullptr.");
        return Status::DB_ERROR;
    }

    if (iKvStoreSnapshot == nullptr) {
        ZLOGE("snapshot is nullptr.");
        return Status::ERROR;
    }

    std::lock_guard<std::mutex> lg(storeSnapshotMutex_);
    Status status = static_cast<KvStoreSnapshotImpl *>(iKvStoreSnapshot.GetRefPtr())->Release(kvStoreDelegate_);
    if (status == Status::SUCCESS) {
        auto it = snapshotMap_.find(static_cast<KvStoreSnapshotImpl *>(iKvStoreSnapshot.GetRefPtr()));
        if (it != snapshotMap_.end()) {
            snapshotMap_.erase(it);
        }
        Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    } else {
        FaultMsg msg = {FaultType::RUNTIME_FAULT, "user", __FUNCTION__, Fault::RF_RELEASE_SNAPSHOT};
        Reporter::GetInstance()->ServiceFault()->Report(msg);
    }
    return status;
}

Status KvStoreImpl::Put(const Key &key, const Value &value)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(key.Data());

    if (keyData.size() == 0 || keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid key.");
        return Status::INVALID_ARGUMENT;
    }

    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }

    DistributedDB::Key dbKey = keyData;
    DistributedDB::Value dbValue = value.Data();
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreDelegate_->Put(dbKey, dbValue);
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("Put failed, distributeddb need recover.");
        return IMPORT_DATABASE(bundleName_);
    }

    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate put failed.");
        return Status::DB_ERROR;
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status KvStoreImpl::PutBatch(const std::vector<Entry> &entries)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }

    // temporary transform.
    std::vector<DistributedDB::Entry> dbEntries;
    for (auto &entry : entries) {
        DistributedDB::Entry dbEntry;

        std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(entry.key.Data());
        if (keyData.size() == 0 || keyData.size() > Constant::MAX_KEY_LENGTH) {
            ZLOGE("invalid key.");
            return Status::INVALID_ARGUMENT;
        }

        dbEntry.key = keyData;
        dbEntry.value = entry.value.Data();
        dbEntries.push_back(dbEntry);
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreDelegate_->PutBatch(dbEntries);
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("PutBatch failed, distributeddb need recover.");
        return IMPORT_DATABASE(bundleName_);
    }

    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate PutBatch failed.");
        return Status::DB_ERROR;
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status KvStoreImpl::Delete(const Key &key)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    if (keyData.size() == 0 || keyData.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGE("invalid key.");
        return Status::INVALID_ARGUMENT;
    }

    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }

    DistributedDB::Key dbKey = keyData;
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreDelegate_->Delete(dbKey);
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("Delete failed, distributeddb need recover.");
        return IMPORT_DATABASE(bundleName_);
    }
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate Delete failed.");
        return Status::DB_ERROR;
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status KvStoreImpl::DeleteBatch(const std::vector<Key> &keys)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }

    // temporary transform.
    std::vector<DistributedDB::Key> dbKeys;
    for (auto &key : keys) {
        std::vector<uint8_t> keyData = Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
        if (keyData.size() == 0 || keyData.size() > Constant::MAX_KEY_LENGTH) {
            ZLOGE("invalid key.");
            return Status::INVALID_ARGUMENT;
        }

        DistributedDB::Key keyTmp = keyData;
        dbKeys.push_back(keyTmp);
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreDelegate_->DeleteBatch(dbKeys);
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("DeleteBatch failed, distributeddb need recover.");
        return IMPORT_DATABASE(bundleName_);
    }
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate DeleteBatch failed.");
        return Status::DB_ERROR;
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status KvStoreImpl::Clear()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreDelegate_->Clear();
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("Clear failed, distributeddb need recover.");
        return IMPORT_DATABASE(bundleName_);
    }
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate Clear failed.");
        return Status::DB_ERROR;
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status KvStoreImpl::StartTransaction()
{
    ZLOGI("begin.");
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreDelegate_->StartTransaction();
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("StartTransaction failed, distributeddb need recover.");
        return IMPORT_DATABASE(bundleName_);
    }
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error.");
        return Status::DB_ERROR;
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status KvStoreImpl::Commit()
{
    ZLOGI("begin.");
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreDelegate_->Commit();
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("Commit failed, distributeddb need recover.");
        return IMPORT_DATABASE(bundleName_);
    }
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error.");
        return Status::DB_ERROR;
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status KvStoreImpl::Rollback()
{
    ZLOGI("begin.");
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreDelegate_->Rollback();
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("Rollback failed, distributeddb need recover.");
        return IMPORT_DATABASE(bundleName_);
    }
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error.");
        return Status::DB_ERROR;
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

InnerStatus KvStoreImpl::Close(DistributedDB::KvStoreDelegateManager *kvStoreDelegateManager)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    ZLOGW("start Close");
    if (openCount_ > 1) {
        openCount_--;
        return InnerStatus::DECREASE_REFCOUNT;
    }
    Status status = ForceClose(kvStoreDelegateManager);
    if (status == Status::SUCCESS) {
        return InnerStatus::SUCCESS;
    }
    return InnerStatus::ERROR;
}

Status KvStoreImpl::ForceClose(DistributedDB::KvStoreDelegateManager *kvStoreDelegateManager)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    ZLOGI("start ForceClose, current openCount is %d.", openCount_);
    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr || kvStoreDelegateManager == nullptr) {
        ZLOGW("close got nullptr");
        return Status::INVALID_ARGUMENT;
    }
    ZLOGI("ForceClose start to clean observer");
    std::lock_guard<std::mutex> observerSetLockGuard(storeObserverMutex_);
    for (auto observer = observerSet_.begin(); observer != observerSet_.end();) {
        DistributedDB::DBStatus dbStatus = kvStoreDelegate_->UnRegisterObserver(*observer);
        if (dbStatus == DistributedDB::DBStatus::OK) {
            delete *observer;
            observer = observerSet_.erase(observer);
        } else {
            ZLOGW("Force close kvstore failed during UnRegisterObserver, status %d.", dbStatus);
            return Status::ERROR;
        }
    }
    ZLOGI("ForceClose start to clean snapshot");
    std::lock_guard<std::mutex> snapshotMapLockGuard(storeSnapshotMutex_);
    for (auto snapshotPair = snapshotMap_.begin(); snapshotPair != snapshotMap_.end();) {
        auto *snapshotImpl = static_cast<KvStoreSnapshotImpl *>(snapshotPair->second.GetRefPtr());
        if (snapshotImpl != nullptr) {
            auto status = snapshotImpl->Release(kvStoreDelegate_);
            if (status != Status::SUCCESS) {
                ZLOGW("Force close kvstore failed during release snapshot, errCode %d", status);
                return status;
            }
        }
        snapshotPair = snapshotMap_.erase(snapshotPair);
    }
    DistributedDB::DBStatus status = kvStoreDelegateManager->CloseKvStore(kvStoreDelegate_);
    if (status == DistributedDB::DBStatus::OK) {
        kvStoreDelegate_ = nullptr;
        ZLOGI("end ForceClose.");
        return Status::SUCCESS;
    }
    ZLOGI("ForceClose close failed with error code %d.", status);
    return Status::ERROR;
}

Status KvStoreImpl::MigrateKvStore(const std::string &harmonyAccountId,
                                   const std::string &kvStoreDataDir,
                                   DistributedDB::KvStoreDelegateManager *oldDelegateMgr,
                                   DistributedDB::KvStoreDelegateManager *&newDelegateMgr)
{
    ZLOGI("begin.");
    std::unique_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (oldDelegateMgr == nullptr) {
        ZLOGW("kvStore delegate manager is nullptr.");
        return Status::INVALID_ARGUMENT;
    }

    ZLOGI("create new KvStore.");
    std::vector<uint8_t> secretKey; // expected get secret key from meta kvstore successful when encrypt flag is true.
    std::unique_ptr<std::vector<uint8_t>, void(*)(std::vector<uint8_t>*)> cleanGuard(
            &secretKey, [](std::vector<uint8_t> *ptr) { ptr->assign(ptr->size(), 0); });
    auto metaSecretKey = KvStoreMetaManager::GetMetaKey(deviceAccountId_, "default", bundleName_, storeId_, "KEY");
    if (options_.encrypt) {
        bool outdated = false; // ignore outdated flag during rebuild kvstore.
        KvStoreMetaManager::GetInstance().GetSecretKeyFromMeta(metaSecretKey, secretKey, outdated);
        if (secretKey.empty()) {
            ZLOGE("Get secret key from meta kvstore failed.");
            return Status::CRYPT_ERROR;
        }
    }

    DistributedDB::KvStoreDelegate::Option dbOption;
    Status status = KvStoreAppManager::InitDbOption(options_, secretKey, dbOption);
    if (status != Status::SUCCESS) {
        ZLOGE("InitDbOption failed.");
        return status;
    }
    if (newDelegateMgr == nullptr) {
        auto appId = KvStoreUtils::GetAppIdByBundleName(bundleName_);
        if (appId.empty()) {
            ZLOGE("Get appId by bundle name failed.");
            return Status::MIGRATION_KVSTORE_FAILED;
        }
        newDelegateMgr = new (std::nothrow) DistributedDB::KvStoreDelegateManager(appId, harmonyAccountId);
        if (newDelegateMgr == nullptr) {
            ZLOGE("new KvStoreDelegateManager failed.");
            return Status::MIGRATION_KVSTORE_FAILED;
        }
        DistributedDB::KvStoreConfig kvStoreConfig {kvStoreDataDir};
        newDelegateMgr->SetKvStoreConfig(kvStoreConfig);
    }

    DistributedDB::DBStatus dbStatus = DistributedDB::DBStatus::OK;
    DistributedDB::KvStoreDelegate *kvStoreDelegate = nullptr; // new KvStoreDelegate get from distributed DB.
    newDelegateMgr->GetKvStore(storeId_, dbOption,
        [&](DistributedDB::DBStatus result, DistributedDB::KvStoreDelegate *delegate) {
            kvStoreDelegate = delegate;
            dbStatus = result;
        });

    if (kvStoreDelegate == nullptr) {
        ZLOGE("storeDelegate is nullptr, dbStatusTmp: %d", static_cast<int>(dbStatus));
        return Status::DB_ERROR;
    }

    status = RebuildKvStoreObserver(kvStoreDelegate);
    if (status != Status::SUCCESS) {
        ZLOGI("rebuild KvStore observer failed, errCode %d.", static_cast<int>(status));
        // skip this failed, continue to do other rebuild process.
    }

    status = RebuildKvStoreSnapshot(kvStoreDelegate);
    if (status != Status::SUCCESS) {
        ZLOGI("rebuild KvStore snapshot failed, errCode %d.", static_cast<int>(status));
        // skip this failed, continue to do close kvstore process.
    }

    ZLOGI("close old KvStore.");
    dbStatus = oldDelegateMgr->CloseKvStore(kvStoreDelegate_);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGI("rebuild KvStore failed during close KvStore, errCode %d.", static_cast<int>(dbStatus));
        newDelegateMgr->CloseKvStore(kvStoreDelegate);
        return Status::DB_ERROR;
    }

    ZLOGI("update kvstore delegate.");
    kvStoreDelegate_ = kvStoreDelegate;
    return Status::SUCCESS;
}

Status KvStoreImpl::RebuildKvStoreObserver(DistributedDB::KvStoreDelegate *kvStoreDelegate)
{
    ZLOGI("rebuild observer.");
    if (kvStoreDelegate_ == nullptr || kvStoreDelegate == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    std::lock_guard<std::mutex> observerSetLockGuard(storeObserverMutex_);
    Status status = Status::SUCCESS;
    DistributedDB::DBStatus dbStatus;
    for (const auto observer : observerSet_) {
        dbStatus = kvStoreDelegate_->UnRegisterObserver(observer);
        if (dbStatus != DistributedDB::OK) {
            status = Status::DB_ERROR;
            ZLOGW("rebuild observer failed during UnRegisterObserver, status %d.", static_cast<int>(dbStatus));
            continue;
        }

        dbStatus = kvStoreDelegate->RegisterObserver(observer);
        if (dbStatus != DistributedDB::OK) {
            status = Status::DB_ERROR;
            ZLOGW("rebuild observer failed during RegisterObserver, status %d.", static_cast<int>(dbStatus));
            continue;
        }
    }
    return status;
}

Status KvStoreImpl::RebuildKvStoreSnapshot(DistributedDB::KvStoreDelegate *kvStoreDelegate)
{
    ZLOGI("rebuild snapshot.");
    if (kvStoreDelegate_ == nullptr || kvStoreDelegate == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    std::lock_guard<std::mutex> snapshotMapLockGuard(storeSnapshotMutex_);
    Status retStatus = Status::SUCCESS;
    for (const auto &snapshotPair : snapshotMap_) {
        auto *snapshot = static_cast<KvStoreSnapshotImpl *>(snapshotPair.second.GetRefPtr());
        if (snapshot == nullptr) {
            continue;
        }
        Status status = snapshot->Release(kvStoreDelegate_);
        if (status != Status::SUCCESS) {
            retStatus = status;
            ZLOGW("rebuild snapshot failed during release snapshot, errCode %d", static_cast<int>(status));
            continue;
        }

        status = snapshot->MigrateKvStore(kvStoreDelegate);
        if (status != Status::SUCCESS) {
            retStatus = status;
            ZLOGW("rebuild snapshot failed during migrate snapshot, errCode %d", static_cast<int>(status));
            continue;
        }
    }
    return retStatus;
}

void KvStoreImpl::IncreaseOpenCount()
{
    openCount_++;
}

/* subscribe kv store */
Status KvStoreImpl::SubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    ZLOGI("begin.");
    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }

    if (observer == nullptr) {
        ZLOGE("observer is nullptr.");
        return Status::INVALID_ARGUMENT;
    }

    std::lock_guard<std::mutex> lg(storeObserverMutex_);
    KvStoreObserverImpl *kvStoreObserverImpl = new (std::nothrow) KvStoreObserverImpl(subscribeType, observer);
    if (kvStoreObserverImpl == nullptr) {
        ZLOGE("kvStoreObserverImpl is nullptr.");
        return Status::ERROR;
    }

    if (observerSet_.find(kvStoreObserverImpl) != observerSet_.end()) {
        ZLOGI("already subscribed.");
        delete kvStoreObserverImpl;
        kvStoreObserverImpl = nullptr;
        return Status::STORE_ALREADY_SUBSCRIBE;
    }

    DistributedDB::DBStatus status = kvStoreDelegate_->RegisterObserver(kvStoreObserverImpl);
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error.");
        delete kvStoreObserverImpl;
        kvStoreObserverImpl = nullptr;
        return Status::DB_ERROR;
    }

    auto it = observerSet_.insert(kvStoreObserverImpl);
    ZLOGI("set size: %zu.", observerSet_.size());
    if (!(it.second)) {
        status = kvStoreDelegate_->UnRegisterObserver(kvStoreObserverImpl);
        ZLOGI("insert failed set size: %zu status: %d.", observerSet_.size(), static_cast<int>(status));
        delete kvStoreObserverImpl;
        kvStoreObserverImpl = nullptr;
        return Status::STORE_ALREADY_SUBSCRIBE;
    } else {
        ZLOGI("insert success set size: %zu.", observerSet_.size());
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

/* unsubscribe kv store */
Status KvStoreImpl::UnSubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    ZLOGI("begin.");
    KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(Status::SYSTEM_ACCOUNT_EVENT_PROCESSING);
    Status status = Status::DB_ERROR;
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return status;
    }

    if (observer == nullptr) {
        ZLOGE("observer is nullptr.");
        return Status::INVALID_ARGUMENT;
    }

    std::lock_guard<std::mutex> lg(storeObserverMutex_);
    KvStoreObserverImpl *kvStoreObserverImpl = new (std::nothrow) KvStoreObserverImpl(subscribeType, observer);
    if (kvStoreObserverImpl == nullptr) {
        ZLOGE("kvStoreObserverImpl is nullptr.");
        return Status::ERROR;
    }

    ZLOGI("set size: %zu.", observerSet_.size());
    auto it = observerSet_.find(kvStoreObserverImpl);
    if (it != observerSet_.end()) {
        DistributedDB::DBStatus dbStatus;
        {
            DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
            dbStatus = kvStoreDelegate_->UnRegisterObserver(*it);
        }
        if (dbStatus == DistributedDB::DBStatus::OK) {
            delete *it;
            observerSet_.erase(it);
            status = Status::SUCCESS;
        }
        ZLOGE("delegate return status: %d.", static_cast<int>(dbStatus));
    } else {
        ZLOGW("No existing observer to unsubscribe. Return success.");
        status = Status::SUCCESS;
    }

    delete kvStoreObserverImpl;
    kvStoreObserverImpl = nullptr;
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    ZLOGI("return status: %d", static_cast<int>(status));
    return status;
}

Status KvStoreImpl::ReKey(const std::vector<uint8_t> &key) {
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    ZLOGI("begin");
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    if (kvStoreDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }
    DistributedDB::CipherPassword password;
    auto status = password.SetValue(key.data(), key.size());
    if (status != DistributedDB::CipherPassword::ErrorCode::OK) {
        ZLOGE("Failed to set the passwd.");
        return Status::DB_ERROR;
    }
    DistributedDB::DBStatus dbStatus;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        dbStatus = kvStoreDelegate_->Rekey(password);
    }
    if (dbStatus == DistributedDB::DBStatus::OK) {
        ZLOGI("succeed");
        return Status::SUCCESS;
    }
    return Status::ERROR;
}

const std::string KvStoreImpl::GetStorePath()
{
    return storePath_;
}

KvStoreImpl::~KvStoreImpl()
{
    ZLOGI("destruct");
}

bool KvStoreImpl::Import(const std::string &bundleName) const
{
    ZLOGI("KvStoreImpl Import start");
    const std::string harmonyAccountId = AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId();
    auto metaSecretKey = KvStoreMetaManager::GetMetaKey(deviceAccountId_, harmonyAccountId, bundleName, storeId_,
                                                        "SINGLE_KEY");
    std::vector<uint8_t> secretKey;
    bool outdated = false;
    auto trueAppId = KvStoreUtils::GetAppIdByBundleName(bundleName);
    KvStoreMetaManager::GetInstance().GetSecretKeyFromMeta(metaSecretKey, secretKey, outdated);

    MetaData metaData{0};
    metaData.kvStoreMetaData.deviceAccountId = deviceAccountId_;
    metaData.kvStoreMetaData.userId = harmonyAccountId;
    metaData.kvStoreMetaData.bundleName = bundleName;
    metaData.kvStoreMetaData.appId = trueAppId;
    metaData.kvStoreMetaData.storeId = storeId_;
    metaData.secretKeyMetaData.secretKey = secretKey;
    std::shared_lock<std::shared_mutex> lock(storeDelegateMutex_);
    return std::make_unique<BackupHandler>()->MultiKvStoreRecover(metaData, kvStoreDelegate_);
}
}  // namespace DistributedKv
}  // namespace OHOS
