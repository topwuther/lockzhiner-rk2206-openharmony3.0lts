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

#define LOG_TAG "AppKvStoreImpl"

#include "app_kvstore_impl.h"
#include <chrono>
#include <mutex>
#include <thread>
#include "app_kvstore_result_set_impl.h"
#include "app_types.h"
#include "log_print.h"
#include "types.h"
#include "reporter.h"

namespace OHOS {
namespace AppDistributedKv {
using namespace std::chrono_literals;
using namespace OHOS::DistributedKv;
AppKvStoreImpl::AppKvStoreImpl(const std::string &storeId, DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate)
    : storeId_(storeId), kvStoreNbDelegate_(kvStoreNbDelegate)
{
    ZLOGI("constructor appkvstore");
}

AppKvStoreImpl::~AppKvStoreImpl()
{
    ZLOGI("destructor appKvStore.");
    // This function will be called after DB close, so every Subscribe/Unsubscribe call will fail and no more element
    // will be added to observer maps.
    {
        std::lock_guard<std::mutex> localLock(localObserverMapMutex_);
        for (auto const &iter : localObserverMap_) {
            delete iter.second;
        }
        localObserverMap_.clear();
    }
    {
        std::lock_guard<std::mutex> syncLock(syncedObserverMapMutex_);
        for (auto const &iter : syncedObserverMap_) {
            delete iter.second;
        }
        syncedObserverMap_.clear();
    }
}

// Get id of this AppKvStore.
const std::string &AppKvStoreImpl::GetStoreId()
{
    return storeId_;
}

// options: Mark this is a local entry or not.
Status AppKvStoreImpl::Put(const WriteOptions &options, const Key &key, const Value &value)
{
    ZLOGD("start");
    auto trimmedKey = DistributedKv::Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    // Restrict key and value size to interface specification.
    if (trimmedKey.size() == 0 || trimmedKey.size() > DistributedKv::Constant::MAX_KEY_LENGTH ||
        value.Data().size() > DistributedKv::Constant::MAX_VALUE_LENGTH) {
        ZLOGW("invalid_argument.");
        return Status::INVALID_ARGUMENT;
    }
    DistributedDB::Key tmpKey = trimmedKey;
    DistributedDB::Value tmpValue = value.Data();
    DistributedDB::DBStatus status;
    if (options.local) {
        status = kvStoreNbDelegate_->PutLocal(tmpKey, tmpValue);
    } else {
        status = kvStoreNbDelegate_->Put(tmpKey, tmpValue);
    }
    if (status == DistributedDB::DBStatus::OK) {
        ZLOGD("put succeed.");
        Reporter::GetInstance()->VisitStatistic()->Report({appId_, __FUNCTION__});
        return Status::SUCCESS;
    }
    ZLOGW("put failed. status: %d.", static_cast<int>(status));
    return ConvertErrorCode(status);
}

// options: mark this delete is a local change or not.
Status AppKvStoreImpl::Delete(const WriteOptions &options, const Key &key)
{
    ZLOGD("start.");
    auto trimmedKey = DistributedKv::Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    if (trimmedKey.size() == 0 || trimmedKey.size() > DistributedKv::Constant::MAX_KEY_LENGTH) {
        ZLOGW("invalid argument.");
        return Status::INVALID_ARGUMENT;
    }
    DistributedDB::Key tmpKey = trimmedKey;
    DistributedDB::DBStatus status;
    if (options.local) {
        status = kvStoreNbDelegate_->DeleteLocal(tmpKey);
    } else {
        status = kvStoreNbDelegate_->Delete(tmpKey);
    }
    if (status == DistributedDB::DBStatus::OK) {
        ZLOGD("succeed.");
        Reporter::GetInstance()->VisitStatistic()->Report({appId_, __FUNCTION__});
        return Status::SUCCESS;
    }
    ZLOGW("failed status: %d.", static_cast<int>(status));
    return ConvertErrorCode(status);
}

// Get value from AppKvStore by its key. Set options->local to true if you want to get from local kvstore.
// Parameters:
//     options: mark we get from local store or remote store. options->batch is a reserved parameter and should
//              always be false.
Status AppKvStoreImpl::Get(const ReadOptions &options, const Key &key, Value &value)
{
    ZLOGD("start");
    auto trimmedKey = DistributedKv::Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    if (trimmedKey.size() == 0 || trimmedKey.size() > DistributedKv::Constant::MAX_KEY_LENGTH) {
        return Status::INVALID_ARGUMENT;
    }
    DistributedDB::Key tmpKey = trimmedKey;
    DistributedDB::Value tmpValue;
    DistributedDB::DBStatus status;
    if (options.local) {
        status = kvStoreNbDelegate_->GetLocal(tmpKey, tmpValue);
    } else {
        status = kvStoreNbDelegate_->Get(tmpKey, tmpValue);
    }
    ZLOGD("status: %d.", static_cast<int>(status));
    // Value don't have other write method.
    Value tmpValueForCopy(tmpValue);
    value = tmpValueForCopy;
    if (status == DistributedDB::DBStatus::OK) {
        Reporter::GetInstance()->VisitStatistic()->Report({appId_, __FUNCTION__});
        return Status::SUCCESS;
    }
    return ConvertErrorCode(status);
}

// Get all entries in this store which key start with prefixKey.
// Parameters:
//     options: mark we get from local store or remote store. options->batch is a reserved parameter and should
//              always be false.
Status AppKvStoreImpl::GetEntries(const Key &prefixKey, std::vector<Entry> &entries)
{
    ZLOGD("start.");
    auto trimmedPrefix = DistributedKv::Constant::TrimCopy<std::vector<uint8_t>>(prefixKey.Data());
    if (trimmedPrefix.size() > DistributedKv::Constant::MAX_KEY_LENGTH) {
        return Status::INVALID_ARGUMENT;
    }
    DistributedDB::Key tmpKeyPrefix = trimmedPrefix;
    std::vector<DistributedDB::Entry> dbEntries;
    DistributedDB::DBStatus status = kvStoreNbDelegate_->GetEntries(tmpKeyPrefix, dbEntries);
    if (status == DistributedDB::DBStatus::OK) {
        entries.reserve(dbEntries.size());
        ZLOGD("vector size: %zu status: %d.", dbEntries.size(), static_cast<int>(status));
        for (auto const &dbEntry : dbEntries) {
            Key tmpKey(dbEntry.key);
            Value tmpValue(dbEntry.value);
            entries.push_back({.key = tmpKey, .value = tmpValue});
        }
        return Status::SUCCESS;
    }
    return ConvertErrorCode(status);
}

// Get all entries in this store which key start with prefixKey.
Status AppKvStoreImpl::GetEntries(const Key &prefixKey, AppKvStoreResultSet *&resultSet)
{
    ZLOGD("start.");
    auto trimmedPrefix = DistributedKv::Constant::TrimCopy<std::vector<uint8_t>>(prefixKey.Data());
    if (trimmedPrefix.size() > DistributedKv::Constant::MAX_KEY_LENGTH) {
        return Status::INVALID_ARGUMENT;
    }
    DistributedDB::Key tmpKeyPrefix = trimmedPrefix;
    DistributedDB::KvStoreResultSet *dbResultSet = nullptr;
    DistributedDB::DBStatus status = kvStoreNbDelegate_->GetEntries(tmpKeyPrefix, dbResultSet);
    if (status == DistributedDB::DBStatus::OK) {
        resultSet = new (std::nothrow) AppKvStoreResultSetImpl(dbResultSet, kvStoreNbDelegate_);
        if (resultSet == nullptr) {
            ZLOGW("new resultSet failed.");
            return Status::ERROR;
        }
        return Status::SUCCESS;
    }
    return ConvertErrorCode(status);
}

// Close the result set returned by GetEntries().
Status AppKvStoreImpl::CloseResultSet(AppKvStoreResultSet *&resultSet)
{
    if (resultSet == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    if (resultSet->Close() == Status::SUCCESS) {
        delete resultSet;
        resultSet = nullptr;
        return Status::SUCCESS;
    }
    return Status::ERROR;
}

// Close this kvstore in KvStoreDelegateManager. This method is called before this store object destruct.
Status AppKvStoreImpl::Close(DistributedDB::KvStoreDelegateManager *kvStoreDelegateManager)
{
    if (kvStoreDelegateManager == nullptr) {
        return Status::INVALID_ARGUMENT;
    }

    DistributedDB::DBStatus status = kvStoreDelegateManager->CloseKvStore(kvStoreNbDelegate_);
    if (status == DistributedDB::DBStatus::OK) {
        VisitStat vs { appId_, __FUNCTION__ };
        Reporter::GetInstance()->VisitStatistic()->Report(vs);
        return Status::SUCCESS;
    }
    return Status::ERROR;
}

// Sync store with other devices. This is an asynchronous method,
// sync will fail if there is a syncing operation in progress.
// Parameters:
//     deviceIdList: device list to sync.
//     mode: mode can be set to SyncMode::PUSH, SyncMode::PULL and SyncMode::PUTH_PULL. PUSH_PULL will firstly
//           push all not-local store to listed devices, then pull these stores back.
//     callback: return <device-id, sync-result> map to caller.
// Return:
//     Status of this Sync operation.
Status AppKvStoreImpl::Sync(const std::vector<std::string> &deviceIdList, const SyncMode &mode,
                            const std::function<void(const std::map<std::string, Status> &)> &callback)
{
    ZLOGD("start.");
    DistributedDB::DBStatus status;
    DistributedDB::SyncMode dbMode;
    if (mode == SyncMode::PUSH) {
        dbMode = DistributedDB::SyncMode::SYNC_MODE_PUSH_ONLY;
    } else if (mode == SyncMode::PULL) {
        dbMode = DistributedDB::SyncMode::SYNC_MODE_PULL_ONLY;
    } else {
        dbMode = DistributedDB::SyncMode::SYNC_MODE_PUSH_PULL;
    }

    bool syncStatus = syncStatus_.try_lock();
    if (!syncStatus) {
        ZLOGW("Another sync operation still in progress, still call sync right now.");
    }
    status = kvStoreNbDelegate_->Sync(deviceIdList, dbMode,
        [&, callback](const std::map<std::string, DistributedDB::DBStatus> &devices) {
            syncStatus_.unlock();
            std::map<std::string, Status> resultMap;
            for (auto device : devices) {
                if (device.second == DistributedDB::DBStatus::OK) {
                    resultMap[device.first] = Status::SUCCESS;
                } else if (device.second == DistributedDB::DBStatus::NOT_FOUND) {
                    resultMap[device.first] = Status::DEVICE_NOT_FOUND;
                } else if (device.second == DistributedDB::DBStatus::TIME_OUT) {
                    resultMap[device.first] = Status::TIME_OUT;
                } else {
                    resultMap[device.first] = Status::ERROR;
                }
            }
            ZLOGD("callback.");
            callback(resultMap);
        });

    ZLOGD("end: %d", static_cast<int>(status));
    Reporter::GetInstance()->VisitStatistic()->Report({appId_, __FUNCTION__});
    if (status == DistributedDB::DBStatus::OK) {
        return Status::SUCCESS;
    }
    return ConvertErrorCode(status);
}

// Register change of this kvstore to a client-defined observer. observer->OnChange method will be called when store
// changes. One observer can subscribe more than one AppKvStore.
// Parameters:
//     options: mark this is a local entry or not.
//     subscribeType: OBSERVER_CHANGES_NATIVE means native changes of syncable kv store,
//                  : OBSERVER_CHANGES_FOREIGN means synced data changes from remote devices,
//                  : OBSERVER_CHANGES_ALL means both native changes and synced data changes.
//     observer: observer to subscribe changes.
// Return:
//     Status of this subscribe operation.
Status AppKvStoreImpl::SubscribeKvStore(const ReadOptions &options, const SubscribeType &subscribeType,
                                        AppKvStoreObserver *observer)
{
    ZLOGD("start.");
    if (observer == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    DistributedDB::DBStatus dbStatus;
    KvStoreObserverNbImpl *nbObserver = new (std::nothrow) KvStoreObserverNbImpl(observer, subscribeType);
    if (nbObserver == nullptr) {
        ZLOGW("new KvStoreObserverNbImpl failed");
        return Status::ERROR;
    }
    DistributedDB::Key emptyKey;
    if (options.local) {
        std::lock_guard<std::mutex> lock(localObserverMapMutex_);
        bool alreadySubscribed = (localObserverMap_.find(observer) != localObserverMap_.end());
        if (alreadySubscribed) {
            delete nbObserver;
            return Status::STORE_ALREADY_SUBSCRIBE;
        }

        dbStatus = kvStoreNbDelegate_->RegisterObserver(
            emptyKey, DistributedDB::ObserverMode::OBSERVER_CHANGES_LOCAL_ONLY, nbObserver);
        if (dbStatus == DistributedDB::DBStatus::OK) {
            localObserverMap_.insert(std::pair<AppKvStoreObserver *, KvStoreObserverNbImpl *>(observer, nbObserver));
        }
    } else {
        std::lock_guard<std::mutex> lock(syncedObserverMapMutex_);
        bool alreadySubscribed = (syncedObserverMap_.find(observer) != syncedObserverMap_.end());
        if (alreadySubscribed) {
            delete nbObserver;
            return Status::STORE_ALREADY_SUBSCRIBE;
        }
        int dbObserverMode;
        if (subscribeType == SubscribeType::OBSERVER_CHANGES_NATIVE) {
            dbObserverMode = DistributedDB::ObserverMode::OBSERVER_CHANGES_NATIVE;
        } else if (subscribeType == SubscribeType::OBSERVER_CHANGES_FOREIGN) {
            dbObserverMode = DistributedDB::ObserverMode::OBSERVER_CHANGES_FOREIGN;
        } else {
            dbObserverMode = DistributedDB::ObserverMode::OBSERVER_CHANGES_FOREIGN |
                             DistributedDB::ObserverMode::OBSERVER_CHANGES_NATIVE;
        }
        dbStatus = kvStoreNbDelegate_->RegisterObserver(emptyKey, dbObserverMode, nbObserver);
        if (dbStatus == DistributedDB::DBStatus::OK) {
            syncedObserverMap_.insert(std::pair<AppKvStoreObserver *, KvStoreObserverNbImpl *>(observer, nbObserver));
        }
    }
    Reporter::GetInstance()->VisitStatistic()->Report({appId_, __FUNCTION__});
    if (dbStatus == DistributedDB::DBStatus::OK) {
        return Status::SUCCESS;
    }

    delete nbObserver;
    if (dbStatus == DistributedDB::DBStatus::INVALID_ARGS) {
        return Status::INVALID_ARGUMENT;
    }
    if (dbStatus == DistributedDB::DBStatus::DB_ERROR) {
        return Status::DB_ERROR;
    }
    return Status::ERROR;
}

// Unregister a kvstore to an observer.
// Parameters:
//     options: mark this is a local entry or not.
//     subscribeType: OBSERVER_CHANGES_NATIVE means native changes of syncable kv store,
//                  : OBSERVER_CHANGES_FOREIGN means synced data changes from remote devices,
//                  : OBSERVER_CHANGES_ALL means both native changes and synced data changes.
//     observer: observer to unsubscribe this store.
// Return:
//     Status of this unsubscribe operation.
Status AppKvStoreImpl::UnSubscribeKvStore(const ReadOptions &options, const SubscribeType &subscribeType,
                                          AppKvStoreObserver *observer)
{
    ZLOGD("start.");
    if (observer == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    DistributedDB::DBStatus dbStatus;
    if (options.local) {
        std::lock_guard<std::mutex> lock(localObserverMapMutex_);
        auto nbObserver = localObserverMap_.find(observer);
        if (nbObserver == localObserverMap_.end()) {
            return Status::STORE_NOT_SUBSCRIBE;
        }
        dbStatus = kvStoreNbDelegate_->UnRegisterObserver(nbObserver->second);
        if (dbStatus == DistributedDB::DBStatus::OK) {
            delete nbObserver->second;
            localObserverMap_.erase(nbObserver);
        }
    } else {
        std::lock_guard<std::mutex> lock(syncedObserverMapMutex_);
        auto nbObserver = syncedObserverMap_.find(observer);
        if (nbObserver == syncedObserverMap_.end()) {
            return Status::STORE_NOT_SUBSCRIBE;
        }
        dbStatus = kvStoreNbDelegate_->UnRegisterObserver(nbObserver->second);
        if (dbStatus == DistributedDB::DBStatus::OK) {
            delete nbObserver->second;
            syncedObserverMap_.erase(nbObserver);
        }
    }
    Reporter::GetInstance()->VisitStatistic()->Report({appId_, __FUNCTION__});
    if (dbStatus == DistributedDB::DBStatus::OK) {
        return Status::SUCCESS;
    }
    if (dbStatus == DistributedDB::DBStatus::NOT_FOUND) {
        return Status::STORE_NOT_SUBSCRIBE;
    }
    if (dbStatus == DistributedDB::DBStatus::INVALID_ARGS) {
        return Status::INVALID_ARGUMENT;
    }
    return Status::ERROR;
}

Status AppKvStoreImpl::RemoveDeviceData(const std::string &device)
{
    ZLOGD("start");
    DistributedDB::DBStatus status = kvStoreNbDelegate_->RemoveDeviceData(device);
    if (status == DistributedDB::DBStatus::OK) {
        return Status::SUCCESS;
    }
    return Status::ERROR;
}

Status AppKvStoreImpl::SetConflictResolutionPolicy(
    AppKvStoreConflictPolicyType appConflictPolicyType,
    std::function<void(const AppKvStoreConflictData &appConflictData)> callback)
{
    ZLOGD("start.");
    int conflictType = static_cast<int>(appConflictPolicyType);
    DistributedDB::DBStatus dbStatus = kvStoreNbDelegate_->SetConflictNotifier(
        conflictType, [callback, this](const DistributedDB::KvStoreNbConflictData &kvStoreNbConflictData) {
            ZLOGD("callback ");
            KvStoreConflictEntry kvstoreConflictEntry;
            FormKvStoreConflictEntry(kvStoreNbConflictData, kvstoreConflictEntry);
            AppKvStoreConflictDataImpl appConflictDataImpl(kvstoreConflictEntry);
            callback(appConflictDataImpl);
        });
    if (dbStatus != DistributedDB::DBStatus::OK) {
        return Status::DB_ERROR;
    }
    return Status::SUCCESS;
}

void AppKvStoreImpl::FormKvStoreConflictEntry(const DistributedDB::KvStoreNbConflictData &data,
                                              KvStoreConflictEntry &kvstoreConflictEntry)
{
    kvstoreConflictEntry.type = data.GetType();
    DistributedDB::Key dbKey;
    data.GetKey(dbKey);
    Key tmpKey(dbKey);
    kvstoreConflictEntry.key = tmpKey;
    FormKvStoreConflictData(data, DistributedDB::KvStoreNbConflictData::ValueType::OLD_VALUE,
                            kvstoreConflictEntry.oldData);
    FormKvStoreConflictData(data, DistributedDB::KvStoreNbConflictData::ValueType::NEW_VALUE,
                            kvstoreConflictEntry.newData);
}

void AppKvStoreImpl::FormKvStoreConflictData(const DistributedDB::KvStoreNbConflictData &data,
                                             DistributedDB::KvStoreNbConflictData::ValueType type,
                                             KvStoreConflictData &kvStoreConflictData)
{
    DistributedDB::DBStatus dbStatus;
    kvStoreConflictData.isLocal = data.IsNative(type);
    kvStoreConflictData.isDeleted = data.IsDeleted(type);
    if (!kvStoreConflictData.isDeleted) {
        DistributedDB::Value dbValue;
        dbStatus = data.GetValue(type, dbValue);
        if (dbStatus != DistributedDB::DBStatus::OK) {
            ZLOGE("Failed to handle conflict, error: bad conflict data");
            kvStoreConflictData.status = Status::DB_ERROR;
            return;
        }
        Value tmpValue(dbValue);
        kvStoreConflictData.status = Status::SUCCESS;
        kvStoreConflictData.value = tmpValue;
    }
}

Status AppKvStoreImpl::Export(const std::string &filePath, const std::vector<uint8_t> &passwd)
{
    ZLOGD("export start");
    if (filePath.empty()) {
        return Status::INVALID_ARGUMENT;
    }
    DistributedDB::CipherPassword password;
    auto status = password.SetValue(passwd.data(), passwd.size());
    if (status != DistributedDB::CipherPassword::ErrorCode::OK) {
        ZLOGE("Failed to set the passwd.");
        return Status::DB_ERROR;
    }
    DistributedDB::DBStatus dbStatus = kvStoreNbDelegate_->Export(filePath, password);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("Failed to export, path:%s", filePath.c_str());
        return Status::DB_ERROR;
    }
    ZLOGD("export end");
    return Status::SUCCESS;
}

Status AppKvStoreImpl::Import(const std::string &filePath, const std::vector<uint8_t> &passwd)
{
    ZLOGD("Import start");
    if (filePath.empty()) {
        return Status::INVALID_ARGUMENT;
    }
    DistributedDB::CipherPassword password;
    auto status = password.SetValue(passwd.data(), passwd.size());
    if (status != DistributedDB::CipherPassword::ErrorCode::OK) {
        ZLOGE("Failed to set the passwd.");
        return Status::DB_ERROR;
    }
    DistributedDB::DBStatus dbStatus = kvStoreNbDelegate_->Import(filePath, password);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("Failed to export, path:%s", filePath.c_str());
        return Status::DB_ERROR;
    }
    ZLOGD("Import end");
    return Status::SUCCESS;
}

Status AppKvStoreImpl::GetSecurityLevel(SecurityLevel &securityLevel) const
{
    ZLOGD("start");
    DistributedDB::SecurityOption option;
    DistributedDB::DBStatus dbStatus = kvStoreNbDelegate_->GetSecurityOption(option);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("Failed to get security level");
        return Status::DB_ERROR;
    }
    switch (option.securityLabel) {
        case DistributedDB::NOT_SET:
        case DistributedDB::S0:
        case DistributedDB::S1:
        case DistributedDB::S2:
            securityLevel = static_cast<SecurityLevel>(option.securityLabel);
            break;
        case DistributedDB::S3:
            securityLevel = option.securityFlag ? S3 : S3_EX;
            break;
        case DistributedDB::S4:
            securityLevel = S4;
            break;
        default:
            break;
    }
    ZLOGD("end");
    return Status::SUCCESS;
}

Status AppKvStoreImpl::ConvertErrorCode(DistributedDB::DBStatus status)
{
    switch (status) {
        case DistributedDB::DBStatus::BUSY:
        case DistributedDB::DBStatus::DB_ERROR:
            return Status::DB_ERROR;
        case DistributedDB::DBStatus::NOT_FOUND:
            return Status::KEY_NOT_FOUND;
        case DistributedDB::DBStatus::INVALID_ARGS:
            return Status::INVALID_ARGUMENT;
        case DistributedDB::DBStatus::EKEYREVOKED_ERROR:
        case DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR:
            return Status::SECURITY_LEVEL_ERROR;
        default:
            break;
    }
    return Status::ERROR;
}
}  // namespace AppDistributedKv
}  // namespace OHOS
