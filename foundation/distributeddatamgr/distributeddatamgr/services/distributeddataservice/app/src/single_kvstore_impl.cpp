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

#define LOG_TAG "SingleKvStoreImpl"

#include "single_kvstore_impl.h"
#include <fstream>
#include "account_delegate.h"
#include "backup_handler.h"
#include "constant.h"
#include "dds_trace.h"
#include "device_kvstore_resultset_impl.h"
#include "device_kvstore_impl.h"
#include "device_kvstore_observer_impl.h"
#include "kvstore_app_manager.h"
#include "kvstore_data_service.h"
#include "kvstore_meta_manager.h"
#include "kvstore_sync_manager.h"
#include "kvstore_utils.h"
#include "ipc_skeleton.h"
#include "log_print.h"
#include "permission_validator.h"
#include "query_helper.h"
#include "reporter.h"

namespace OHOS::DistributedKv {
static bool TaskIsBackground(pid_t pid)
{
    std::ifstream ifs("/proc/" + std::to_string(pid) + "/cgroup", std::ios::in);
    ZLOGD("pid %d open %d", pid, ifs.good());
    if (!ifs.good()) {
        return false;
    }

    while (!ifs.eof()) {
        const int cgroupLineLen = 256; // enough
        char buffer[cgroupLineLen] = { 0 };
        ifs.getline(buffer, sizeof(buffer));
        std::string line = buffer;

        size_t pos = line.find("background");
        if (pos != std::string::npos) {
            ifs.close();
            return true;
        }
    }
    ifs.close();
    return false;
}

SingleKvStoreImpl::~SingleKvStoreImpl()
{
    RemoveAllSyncOperation();
    ZLOGI("destructor");
}

SingleKvStoreImpl::SingleKvStoreImpl(const Options &options, const std::string &deviceAccountId,
    const std::string &bundleName, const std::string &storeId,
    const std::string &appDirectory, DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate)
    : options_(options),
      deviceAccountId_(deviceAccountId),
      bundleName_(bundleName),
      storeId_(storeId),
      storePath_(Constant::Concatenate({ appDirectory, storeId })),
      kvStoreNbDelegate_(kvStoreNbDelegate),
      observerMapMutex_(),
      observerMap_(),
      storeResultSetMutex_(),
      storeResultSetMap_(),
      openCount_(1),
      flowCtrlManager_(BURST_CAPACITY, SUSTAINED_CAPACITY)
{
}

Status SingleKvStoreImpl::Put(const Key &key, const Value &value)
{
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    auto trimmedKey = Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    // Restrict key and value size to interface specification.
    if (trimmedKey.size() == 0 || trimmedKey.size() > Constant::MAX_KEY_LENGTH ||
        value.Size() > Constant::MAX_VALUE_LENGTH) {
        ZLOGW("invalid_argument.");
        return Status::INVALID_ARGUMENT;
    }
    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        return Status::ILLEGAL_STATE;
    }
    DistributedDB::Key tmpKey = trimmedKey;
    DistributedDB::Value tmpValue = value.Data();
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->Put(tmpKey, tmpValue);
    }
    if (status == DistributedDB::DBStatus::OK) {
        ZLOGD("succeed.");
        return Status::SUCCESS;
    }
    ZLOGW("failed status: %d.", static_cast<int>(status));

    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("Put failed, distributeddb need recover.");
        return (Import(bundleName_) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED);
    }

    return ConvertDbStatus(status);
}

Status SingleKvStoreImpl::ConvertDbStatus(DistributedDB::DBStatus status)
{
    switch (status) {
        case DistributedDB::DBStatus::OK:
            return Status::SUCCESS;
        case DistributedDB::DBStatus::INVALID_ARGS:
            return Status::INVALID_ARGUMENT;
        case DistributedDB::DBStatus::NOT_FOUND:
            return Status::KEY_NOT_FOUND;
        case DistributedDB::DBStatus::INVALID_VALUE_FIELDS:
            return Status::INVALID_VALUE_FIELDS;
        case DistributedDB::DBStatus::INVALID_FIELD_TYPE:
            return Status::INVALID_FIELD_TYPE;
        case DistributedDB::DBStatus::CONSTRAIN_VIOLATION:
            return Status::CONSTRAIN_VIOLATION;
        case DistributedDB::DBStatus::INVALID_FORMAT:
            return Status::INVALID_FORMAT;
        case DistributedDB::DBStatus::INVALID_QUERY_FORMAT:
            return Status::INVALID_QUERY_FORMAT;
        case DistributedDB::DBStatus::INVALID_QUERY_FIELD:
            return Status::INVALID_QUERY_FIELD;
        case DistributedDB::DBStatus::NOT_SUPPORT:
            return Status::NOT_SUPPORT;
        case DistributedDB::DBStatus::EKEYREVOKED_ERROR: // fallthrough
        case DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR:
            return Status::SECURITY_LEVEL_ERROR;
        default:
            break;
    }
    return Status::ERROR;
}

Status SingleKvStoreImpl::Delete(const Key &key)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    auto trimmedKey = DistributedKv::Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    if (trimmedKey.size() == 0 || trimmedKey.size() > Constant::MAX_KEY_LENGTH) {
        ZLOGW("invalid argument.");
        return Status::INVALID_ARGUMENT;
    }
    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        return Status::ILLEGAL_STATE;
    }
    DistributedDB::Key tmpKey = trimmedKey;
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->Delete(tmpKey);
    }
    if (status == DistributedDB::DBStatus::OK) {
        ZLOGD("succeed.");
        return Status::SUCCESS;
    }
    ZLOGW("failed status: %d.", static_cast<int>(status));
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("Delete failed, distributeddb need recover.");
        return (Import(bundleName_) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED);
    }
    return ConvertDbStatus(status);
}

Status SingleKvStoreImpl::Get(const Key &key, Value &value)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    auto trimmedKey = DistributedKv::Constant::TrimCopy<std::vector<uint8_t>>(key.Data());
    if (trimmedKey.empty() || trimmedKey.size() > DistributedKv::Constant::MAX_KEY_LENGTH) {
        return Status::INVALID_ARGUMENT;
    }
    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        return Status::ILLEGAL_STATE;
    }
    DistributedDB::Key tmpKey = trimmedKey;
    DistributedDB::Value tmpValue;
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->Get(tmpKey, tmpValue);
    }
    ZLOGD("status: %d.", static_cast<int>(status));
    if (status == DistributedDB::DBStatus::OK) {
        Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
        // Value don't have other write method.
        Value tmpValueForCopy(tmpValue);
        value = tmpValueForCopy;
        return Status::SUCCESS;
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGI("Get failed, distributeddb need recover.");
        return (Import(bundleName_) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED);
    }
    return ConvertDbStatus(status);
}

Status SingleKvStoreImpl::SubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    return SubscribeKvStore(subscribeType, observer, false);
}

Status SingleKvStoreImpl::SubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer,
                                           bool deviceCoordinate)
{
    ZLOGD("start.");
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    if (observer == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    std::shared_lock<std::shared_mutex> sharedLock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        return Status::ILLEGAL_STATE;
    }
    KvStoreObserverImpl *nbObserver = new (std::nothrow) DeviceKvStoreObserverImpl(subscribeType, observer,
                                                                                   deviceCoordinate);
    if (nbObserver == nullptr) {
        ZLOGW("new KvStoreObserverNbImpl failed");
        return Status::ERROR;
    }

    std::lock_guard<std::mutex> lock(observerMapMutex_);
    IRemoteObject *objectPtr = observer->AsObject().GetRefPtr();
    bool alreadySubscribed = (observerMap_.find(objectPtr) != observerMap_.end());
    if (alreadySubscribed) {
        delete nbObserver;
        return Status::STORE_ALREADY_SUBSCRIBE;
    }
    int dbObserverMode = ConvertToDbObserverMode(subscribeType);
    DistributedDB::Key emptyKey;
    DistributedDB::DBStatus dbStatus;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        dbStatus = kvStoreNbDelegate_->RegisterObserver(emptyKey, dbObserverMode, nbObserver);
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    if (dbStatus == DistributedDB::DBStatus::OK) {
        observerMap_.insert(std::pair<IRemoteObject *, KvStoreObserverImpl *>(objectPtr, nbObserver));
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

// Convert KvStore subscribe type to DistributeDB observer mode.
int SingleKvStoreImpl::ConvertToDbObserverMode(const SubscribeType subscribeType) const
{
    int dbObserverMode;
    if (subscribeType == SubscribeType::SUBSCRIBE_TYPE_LOCAL) {
        dbObserverMode = DistributedDB::OBSERVER_CHANGES_NATIVE;
    } else if (subscribeType == SubscribeType::SUBSCRIBE_TYPE_REMOTE) {
        dbObserverMode = DistributedDB::OBSERVER_CHANGES_FOREIGN;
    } else {
        dbObserverMode = DistributedDB::OBSERVER_CHANGES_FOREIGN | DistributedDB::OBSERVER_CHANGES_NATIVE;
    }
    return dbObserverMode;
}

Status SingleKvStoreImpl::UnSubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    if (observer == nullptr) {
        ZLOGW("observer invalid.");
        return Status::INVALID_ARGUMENT;
    }
    std::shared_lock<std::shared_mutex> sharedLock(storeNbDelegateMutex_);
    std::lock_guard<std::mutex> lock(observerMapMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        return Status::ILLEGAL_STATE;
    }
    IRemoteObject *objectPtr = observer->AsObject().GetRefPtr();
    auto nbObserver = observerMap_.find(objectPtr);
    if (nbObserver == observerMap_.end()) {
        ZLOGW("No existing observer to unsubscribe. Return success.");
        return Status::SUCCESS;
    }
    DistributedDB::DBStatus dbStatus;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        dbStatus = kvStoreNbDelegate_->UnRegisterObserver(nbObserver->second);
    }
    if (dbStatus == DistributedDB::DBStatus::OK) {
        delete nbObserver->second;
        observerMap_.erase(objectPtr);
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    if (dbStatus == DistributedDB::DBStatus::OK) {
        return Status::SUCCESS;
    }

    ZLOGW("failed code=%d.", static_cast<int>(dbStatus));
    if (dbStatus == DistributedDB::DBStatus::NOT_FOUND) {
        return Status::STORE_NOT_SUBSCRIBE;
    }
    if (dbStatus == DistributedDB::DBStatus::INVALID_ARGS) {
        return Status::INVALID_ARGUMENT;
    }
    return Status::ERROR;
}

Status SingleKvStoreImpl::GetEntries(const Key &prefixKey, std::vector<Entry> &entries)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    auto trimmedPrefix = Constant::TrimCopy<std::vector<uint8_t>>(prefixKey.Data());
    if (trimmedPrefix.size() > Constant::MAX_KEY_LENGTH) {
        return Status::INVALID_ARGUMENT;
    }
    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        return Status::ILLEGAL_STATE;
    }
    DistributedDB::Key tmpKeyPrefix = trimmedPrefix;
    std::vector<DistributedDB::Entry> dbEntries;
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->GetEntries(tmpKeyPrefix, dbEntries);
    }
    ZLOGI("result DBStatus: %d", static_cast<int>(status));
    if (status == DistributedDB::DBStatus::OK) {
        entries.reserve(dbEntries.size());
        ZLOGD("vector size: %zu status: %d.", dbEntries.size(), static_cast<int>(status));
        for (auto const &dbEntry : dbEntries) {
            Key tmpKey(dbEntry.key);
            Value tmpValue(dbEntry.value);
            Entry entry;
            entry.key = tmpKey;
            entry.value = tmpValue;
            entries.push_back(entry);
        }
        return Status::SUCCESS;
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGE("GetEntries failed, distributeddb need recover.");
        return (Import(bundleName_) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED);
    }
    if (status == DistributedDB::DBStatus::BUSY || status == DistributedDB::DBStatus::DB_ERROR) {
        return Status::DB_ERROR;
    }
    if (status == DistributedDB::DBStatus::NOT_FOUND) {
        ZLOGI("DB return NOT_FOUND, no matching result. Return success with empty list.");
        return Status::SUCCESS;
    }
    if (status == DistributedDB::DBStatus::INVALID_ARGS) {
        return Status::INVALID_ARGUMENT;
    }
    if (status == DistributedDB::DBStatus::EKEYREVOKED_ERROR ||
        status == DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR) {
        return Status::SECURITY_LEVEL_ERROR;
    }
    return Status::ERROR;
}

Status SingleKvStoreImpl::GetEntriesWithQuery(const std::string &query, std::vector<Entry> &entries)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    ZLOGI("begin");
    bool isSuccess = false;
    DistributedDB::Query dbQuery = QueryHelper::StringToDbQuery(query, isSuccess);
    if (!isSuccess) {
        ZLOGE("StringToDbQuery failed.");
        return Status::INVALID_ARGUMENT;
    } else {
        ZLOGD("StringToDbQuery success.");
    }
    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        return Status::ILLEGAL_STATE;
    }
    std::vector<DistributedDB::Entry> dbEntries;
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->GetEntries(dbQuery, dbEntries);
    }
    ZLOGI("result DBStatus: %d", static_cast<int>(status));
    if (status == DistributedDB::DBStatus::OK) {
        entries.reserve(dbEntries.size());
        ZLOGD("vector size: %zu status: %d.", dbEntries.size(), static_cast<int>(status));
        for (auto const &dbEntry : dbEntries) {
            Key tmpKey(dbEntry.key);
            Value tmpValue(dbEntry.value);
            Entry entry;
            entry.key = tmpKey;
            entry.value = tmpValue;
            entries.push_back(entry);
        }
        return Status::SUCCESS;
    }
    switch (status) {
        case DistributedDB::DBStatus::BUSY:
        case DistributedDB::DBStatus::DB_ERROR: {
            return Status::DB_ERROR;
        }
        case DistributedDB::DBStatus::INVALID_ARGS: {
            return Status::INVALID_ARGUMENT;
        }
        case DistributedDB::DBStatus::INVALID_QUERY_FORMAT: {
            return Status::INVALID_QUERY_FORMAT;
        }
        case DistributedDB::DBStatus::INVALID_QUERY_FIELD: {
            return Status::INVALID_QUERY_FIELD;
        }
        case DistributedDB::DBStatus::NOT_SUPPORT: {
            return Status::NOT_SUPPORT;
        }
        case DistributedDB::DBStatus::NOT_FOUND: {
            ZLOGI("DB return NOT_FOUND, no matching result. Return success with empty list.");
            return Status::SUCCESS;
        }
        case DistributedDB::DBStatus::EKEYREVOKED_ERROR: // fallthrough
        case DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR:
            return Status::SECURITY_LEVEL_ERROR;
        default: {
            return Status::ERROR;
        }
    }
}

void SingleKvStoreImpl::GetResultSet(const Key &prefixKey,
                                     std::function<void(Status, sptr<IKvStoreResultSet>)> callback,
                                     bool deviceCoordinate)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        callback(Status::EXCEED_MAX_ACCESS_RATE, nullptr);
        return;
    }
    auto trimmedPrefix = Constant::TrimCopy<std::vector<uint8_t>>(prefixKey.Data());
    if (trimmedPrefix.size() > Constant::MAX_KEY_LENGTH) {
        callback(Status::INVALID_ARGUMENT, nullptr);
        return;
    }

    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        callback(Status::ILLEGAL_STATE, nullptr);
        return;
    }
    DistributedDB::Key tmpKeyPrefix = trimmedPrefix;
    DistributedDB::KvStoreResultSet *dbResultSet = nullptr;
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->GetEntries(tmpKeyPrefix, dbResultSet);
    }
    ZLOGI("result DBStatus: %d", static_cast<int>(status));
    if (status == DistributedDB::DBStatus::OK) {
        std::lock_guard<std::mutex> lg(storeResultSetMutex_);
        KvStoreResultSetImpl *storeResultSetImpl = new DeviceKvStoreResultSetImpl(tmpKeyPrefix, dbResultSet,
                                                                                  deviceCoordinate);
        sptr<IKvStoreResultSet> storeResultSet = storeResultSetImpl;
        callback(Status::SUCCESS, storeResultSet);
        storeResultSetMap_.emplace(storeResultSetImpl, storeResultSet);
        return;
    }
    switch (status) {
        case DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB: {
            ZLOGE("GetResultSet failed, distributeddb need recover.");
            bool success = Import(bundleName_);
            callback(success ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED, nullptr);
            break;
        }
        case DistributedDB::DBStatus::BUSY: // fallthrough
        case DistributedDB::DBStatus::DB_ERROR:
            callback(Status::DB_ERROR, nullptr);
            break;
        case DistributedDB::DBStatus::NOT_FOUND:
            callback(Status::KEY_NOT_FOUND, nullptr);
            break;
        case DistributedDB::DBStatus::INVALID_ARGS:
            callback(Status::INVALID_ARGUMENT, nullptr);
            break;
        case DistributedDB::DBStatus::EKEYREVOKED_ERROR:
        case DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR:
            callback(Status::SECURITY_LEVEL_ERROR, nullptr);
            break;
        default:
            callback(Status::ERROR, nullptr);
            break;
    }
}

void SingleKvStoreImpl::GetResultSet(const Key &prefixKey,
                                     std::function<void(Status, sptr<IKvStoreResultSet>)> callback)
{
    GetResultSet(prefixKey, callback, false);
}

void SingleKvStoreImpl::GetResultSetWithQuery(const std::string &query,
                                              std::function<void(Status, sptr<IKvStoreResultSet>)> callback)
{
    GetResultSetWithQuery(query, callback, false);
}

void SingleKvStoreImpl::GetResultSetWithQuery(const std::string &query,
                                              std::function<void(Status, sptr<IKvStoreResultSet>)> callback,
                                              bool deviceCoordinate)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        callback(Status::EXCEED_MAX_ACCESS_RATE, nullptr);
        return;
    }
    bool isSuccess = false;
    DistributedDB::Query dbQuery = QueryHelper::StringToDbQuery(query, isSuccess);
    if (!isSuccess) {
        ZLOGE("StringToDbQuery failed.");
        return;
    } else {
        ZLOGD("StringToDbQuery success.");
    }
    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        callback(Status::ILLEGAL_STATE, nullptr);
        return;
    }
    DistributedDB::KvStoreResultSet *dbResultSet = nullptr;
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->GetEntries(dbQuery, dbResultSet);
    }
    ZLOGI("result DBStatus: %d", static_cast<int>(status));
    if (status == DistributedDB::DBStatus::OK) {
        std::lock_guard<std::mutex> lg(storeResultSetMutex_);
        KvStoreResultSetImpl *storeResultSetImpl = new DeviceKvStoreResultSetImpl(dbResultSet, deviceCoordinate);
        sptr<IKvStoreResultSet> storeResultSet = storeResultSetImpl;
        callback(Status::SUCCESS, storeResultSet);
        storeResultSetMap_.emplace(storeResultSetImpl, storeResultSet);
        return;
    }
    switch (status) {
        case DistributedDB::DBStatus::BUSY:
        case DistributedDB::DBStatus::DB_ERROR: {
            callback(Status::DB_ERROR, nullptr);
            break;
        }
        case DistributedDB::DBStatus::INVALID_ARGS: {
            callback(Status::INVALID_ARGUMENT, nullptr);
            break;
        }
        case DistributedDB::DBStatus::INVALID_QUERY_FORMAT: {
            callback(Status::INVALID_QUERY_FORMAT, nullptr);
            break;
        }
        case DistributedDB::DBStatus::INVALID_QUERY_FIELD: {
            callback(Status::INVALID_QUERY_FIELD, nullptr);
            break;
        }
        case DistributedDB::DBStatus::NOT_SUPPORT: {
            callback(Status::NOT_SUPPORT, nullptr);
            break;
        }
        case DistributedDB::DBStatus::EKEYREVOKED_ERROR: // fallthrough
        case DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR:
            callback(Status::SECURITY_LEVEL_ERROR, nullptr);
            break;
        default: {
            callback(Status::ERROR, nullptr);
            break;
        }
    }
}

Status SingleKvStoreImpl::GetCountWithQuery(const std::string &query, int &result)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    ZLOGI("begin");
    bool isSuccess = false;
    DistributedDB::Query dbQuery = QueryHelper::StringToDbQuery(query, isSuccess);
    if (!isSuccess) {
        ZLOGE("StringToDbQuery failed.");
        return Status::INVALID_ARGUMENT;
    } else {
        ZLOGD("StringToDbQuery success.");
    }
    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        return Status::ILLEGAL_STATE;
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->GetCount(dbQuery, result);
    }
    ZLOGI("result DBStatus: %d", static_cast<int>(status));
    switch (status) {
        case DistributedDB::DBStatus::OK: {
            return Status::SUCCESS;
        }
        case DistributedDB::DBStatus::BUSY:
        case DistributedDB::DBStatus::DB_ERROR: {
            return Status::DB_ERROR;
        }
        case DistributedDB::DBStatus::INVALID_ARGS: {
            return Status::INVALID_ARGUMENT;
        }
        case DistributedDB::DBStatus::INVALID_QUERY_FORMAT: {
            return Status::INVALID_QUERY_FORMAT;
        }
        case DistributedDB::DBStatus::INVALID_QUERY_FIELD: {
            return Status::INVALID_QUERY_FIELD;
        }
        case DistributedDB::DBStatus::NOT_SUPPORT: {
            return Status::NOT_SUPPORT;
        }
        case DistributedDB::DBStatus::NOT_FOUND: {
            ZLOGE("DB return NOT_FOUND, no matching result. Return success with count 0.");
            result = 0;
            return Status::SUCCESS;
        }
        case DistributedDB::DBStatus::EKEYREVOKED_ERROR: // fallthrough
        case DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR:
            return Status::SECURITY_LEVEL_ERROR;
        default: {
            return Status::ERROR;
        }
    }
}

Status SingleKvStoreImpl::CloseResultSet(sptr<IKvStoreResultSet> resultSet)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (resultSet == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    std::lock_guard<std::mutex> lg(storeResultSetMutex_);
    KvStoreResultSetImpl *kvStoreResultSetImpl = static_cast<KvStoreResultSetImpl *>(resultSet.GetRefPtr());
    Status status;
    auto it = storeResultSetMap_.find(kvStoreResultSetImpl);
    if (it == storeResultSetMap_.end()) {
        ZLOGE("ResultSet not found in this store.");
        return Status::INVALID_ARGUMENT;
    }
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreResultSetImpl->CloseResultSet(kvStoreNbDelegate_);
    }
    if (status == Status::SUCCESS) {
        storeResultSetMap_.erase(it);
    } else {
        ZLOGE("CloseResultSet failed.");
    }
    return status;
}

Status SingleKvStoreImpl::RemoveDeviceData(const std::string &device)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    // map UUID to UDID
    std::string deviceUDID = KvStoreUtils::GetProviderInstance().GetUuidByNodeId(device);
    if (deviceUDID.empty()) {
        ZLOGE("can't get nodeid");
        return Status::ERROR;
    }
    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("kvstore is not open");
        return Status::ILLEGAL_STATE;
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->RemoveDeviceData(deviceUDID);
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGE("RemoveDeviceData failed, distributeddb need recover.");
        return (Import(bundleName_) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED);
    }
    if (status == DistributedDB::DBStatus::OK) {
        return Status::SUCCESS;
    }
    if (status == DistributedDB::DBStatus::EKEYREVOKED_ERROR ||
        status == DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR) {
        return Status::SECURITY_LEVEL_ERROR;
    }
    return Status::ERROR;
}

Status SingleKvStoreImpl::Sync(const std::vector<std::string> &deviceIdList, const SyncMode &mode,
                               uint32_t allowedDelayMs)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    ZLOGD("start.");
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    uint32_t delayMs = GetSyncDelayTime(allowedDelayMs);
    {
        std::unique_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
        if ((waitingSyncCount_ > 0) &&
            (lastSyncDeviceIdList_ == deviceIdList) && (lastSyncMode_ == mode) && (lastSyncDelayMs_ == delayMs)) {
            return Status::SUCCESS;
        }
        lastSyncDeviceIdList_ = deviceIdList;
        lastSyncMode_ = mode;
        lastSyncDelayMs_ = delayMs;
    }
    return AddSync(deviceIdList, mode, delayMs);
}

Status SingleKvStoreImpl::AddSync(const std::vector<std::string> &deviceIdList, const SyncMode &mode,
                                  uint32_t delayMs)
{
    ZLOGD("start.");
    waitingSyncCount_++;
    return KvStoreSyncManager::GetInstance()->AddSyncOperation(reinterpret_cast<uintptr_t>(this), delayMs,
        std::bind(&SingleKvStoreImpl::DoSync, this, deviceIdList, mode, std::placeholders::_1),
        std::bind(&SingleKvStoreImpl::DoSyncComplete, this, std::placeholders::_1));
}

uint32_t SingleKvStoreImpl::GetSyncDelayTime(uint32_t allowedDelayMs) const
{
    uint32_t delayMs = allowedDelayMs;
    if (delayMs == 0) {
        bool isBackground = TaskIsBackground(IPCSkeleton::GetCallingPid());
        if (isBackground) {
            // delay schedule
            delayMs = defaultSyncDelayMs_ ? defaultSyncDelayMs_ : KvStoreSyncManager::SYNC_DEFAULT_DELAY_MS;
        }
    } else {
        if (delayMs < KvStoreSyncManager::SYNC_MIN_DELAY_MS) {
            delayMs = KvStoreSyncManager::SYNC_MIN_DELAY_MS;
        }
        if (delayMs > KvStoreSyncManager::SYNC_MAX_DELAY_MS) {
            delayMs = KvStoreSyncManager::SYNC_MAX_DELAY_MS;
        }
    }
    return delayMs;
}

Status SingleKvStoreImpl::RemoveAllSyncOperation()
{
    return KvStoreSyncManager::GetInstance()->RemoveSyncOperation(reinterpret_cast<uintptr_t>(this));
}

void SingleKvStoreImpl::DoSyncComplete(const std::map<std::string, DistributedDB::DBStatus> &devicesSyncResult)
{
    DdsTrace trace(std::string("DdsTrace " LOG_TAG "::") + std::string(__FUNCTION__));
    std::map<std::string, Status> resultMap;
    for (auto device : devicesSyncResult) {
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
    syncRetries_ = 0;
    ZLOGD("callback.");
    if (syncCallback_ != nullptr) {
        syncCallback_->SyncCompleted(resultMap);
    }
}

Status SingleKvStoreImpl::DoSync(const std::vector<std::string> &deviceIdList, const SyncMode &mode,
                                 const KvStoreSyncManager::SyncEnd &syncEnd)
{
    ZLOGD("start.");
    std::vector<std::string> deviceUuidList;
    for (auto const &device : deviceIdList) {
        std::string nodeid = KvStoreUtils::GetProviderInstance().GetUuidByNodeId(device);
        if (!nodeid.empty()) {
            deviceUuidList.push_back(nodeid);
        } else {
            ZLOGW("invalid deviceId:%s.", KvStoreUtils::ToBeAnonymous(device).c_str());
        }
    }
    if (deviceUuidList.empty()) {
        ZLOGE("not found deviceIds.");
        return Status::ERROR;
    }
    DistributedDB::SyncMode dbMode;
    if (mode == SyncMode::PUSH) {
        dbMode = DistributedDB::SyncMode::SYNC_MODE_PUSH_ONLY;
    } else if (mode == SyncMode::PULL) {
        dbMode = DistributedDB::SyncMode::SYNC_MODE_PULL_ONLY;
    } else {
        dbMode = DistributedDB::SyncMode::SYNC_MODE_PUSH_PULL;
    }

    DistributedDB::DBStatus status;
    {
        std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
        if (kvStoreNbDelegate_ == nullptr) {
            ZLOGE("kvstore is not open");
            return Status::ILLEGAL_STATE;
        }
        waitingSyncCount_--;
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->Sync(deviceUuidList, dbMode, syncEnd);
        ZLOGD("end: %d", static_cast<int>(status));
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    if (status == DistributedDB::DBStatus::OK) {
        return Status::SUCCESS;
    }
    if (status == DistributedDB::DBStatus::BUSY) {
        if (syncRetries_ < KvStoreSyncManager::SYNC_RETRY_MAX_COUNT) {
            syncRetries_++;
            auto addStatus = AddSync(deviceUuidList, mode, KvStoreSyncManager::SYNC_DEFAULT_DELAY_MS);
            if (addStatus == Status::SUCCESS) {
                return addStatus;
            }
        }
    }
    if (status == DistributedDB::DBStatus::DB_ERROR) {
        return Status::DB_ERROR;
    }
    if (status == DistributedDB::DBStatus::NOT_FOUND) {
        return Status::DEVICE_NOT_FOUND;
    }
    if (status == DistributedDB::DBStatus::INVALID_ARGS) {
        return Status::INVALID_ARGUMENT;
    }
    return Status::ERROR;
}

InnerStatus SingleKvStoreImpl::Close(DistributedDB::KvStoreDelegateManager *kvStoreDelegateManager)
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

Status SingleKvStoreImpl::ForceClose(DistributedDB::KvStoreDelegateManager *kvStoreDelegateManager)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    ZLOGI("start, current openCount is %d.", openCount_);
    std::unique_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr || kvStoreDelegateManager == nullptr) {
        ZLOGW("get nullptr");
        return Status::INVALID_ARGUMENT;
    }
    RemoveAllSyncOperation();
    ZLOGI("start to clean observer");
    std::lock_guard<std::mutex> observerMapLockGuard(observerMapMutex_);
    for (auto observer = observerMap_.begin(); observer != observerMap_.end();) {
        DistributedDB::DBStatus dbStatus = kvStoreNbDelegate_->UnRegisterObserver(observer->second);
        if (dbStatus == DistributedDB::DBStatus::OK) {
            delete observer->second;
            observer = observerMap_.erase(observer);
        } else {
            ZLOGW("UnSubscribeKvStore failed during ForceClose, status %d.", dbStatus);
            return Status::ERROR;
        }
    }
    ZLOGI("start to clean resultset");
    std::lock_guard<std::mutex> lg(storeResultSetMutex_);
    for (auto resultSetPair = storeResultSetMap_.begin(); resultSetPair != storeResultSetMap_.end();) {
        Status status = (resultSetPair->first)->CloseResultSet(kvStoreNbDelegate_);
        if (status != Status::SUCCESS) {
            ZLOGW("CloseResultSet failed during ForceClose, errCode %d", status);
            return status;
        }
        resultSetPair = storeResultSetMap_.erase(resultSetPair);
    }
    DistributedDB::DBStatus status = kvStoreDelegateManager->CloseKvStore(kvStoreNbDelegate_);
    if (status == DistributedDB::DBStatus::OK) {
        kvStoreNbDelegate_ = nullptr;
        ZLOGI("end.");
        return Status::SUCCESS;
    }
    ZLOGI("failed with error code %d.", status);
    return Status::ERROR;
}

Status SingleKvStoreImpl::MigrateKvStore(const std::string &harmonyAccountId,
                                         const std::string &kvStoreDataDir,
                                         DistributedDB::KvStoreDelegateManager *oldDelegateMgr,
                                         DistributedDB::KvStoreDelegateManager *&newDelegateMgr)
{
    ZLOGI("begin.");
    std::unique_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (oldDelegateMgr == nullptr) {
        ZLOGW("kvStore delegate manager is nullptr.");
        return Status::INVALID_ARGUMENT;
    }

    ZLOGI("create new KvStore.");
    std::vector<uint8_t> secretKey; // expected get secret key from meta kvstore successful when encrypt flag is true.
    std::unique_ptr<std::vector<uint8_t>, void(*)(std::vector<uint8_t>*)> cleanGuard(
            &secretKey, [](std::vector<uint8_t> *ptr) { ptr->assign(ptr->size(), 0); });
    bool outdated = false; // ignore outdated flag during rebuild kvstore.
    auto metaSecretKey = KvStoreMetaManager::GetMetaKey(deviceAccountId_, "default", bundleName_, storeId_,
                                                        "SINGLE_KEY");
    if (options_.encrypt) {
        KvStoreMetaManager::GetInstance().GetSecretKeyFromMeta(metaSecretKey, secretKey, outdated);
        if (secretKey.empty()) {
            ZLOGE("Get secret key from meta kvstore failed.");
            return Status::CRYPT_ERROR;
        }
    }

    DistributedDB::DBStatus dbStatus;
    DistributedDB::KvStoreNbDelegate::Option dbOption;
    Status status = KvStoreAppManager::InitNbDbOption(options_, secretKey, dbOption);
    if (status != Status::SUCCESS) {
        ZLOGE("InitNbDbOption failed.");
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
        DistributedDB::KvStoreConfig kvStoreConfig;
        kvStoreConfig.dataDir = kvStoreDataDir;
        newDelegateMgr->SetKvStoreConfig(kvStoreConfig);
    }
    DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate = nullptr; // new KvStoreNbDelegate get from distributed DB.
    newDelegateMgr->GetKvStore(
        storeId_, dbOption,
        [&](DistributedDB::DBStatus status, DistributedDB::KvStoreNbDelegate *delegate) {
            kvStoreNbDelegate = delegate;
            dbStatus = status;
        });
    if (kvStoreNbDelegate == nullptr) {
        ZLOGE("storeDelegate is nullptr, dbStatusTmp: %d", static_cast<int>(dbStatus));
        return Status::DB_ERROR;
    }

    if (options_.autoSync) {
        bool autoSync = true;
        auto data = static_cast<DistributedDB::PragmaData>(&autoSync);
        auto pragmaStatus = kvStoreNbDelegate->Pragma(DistributedDB::PragmaCmd::AUTO_SYNC, data);
        if (pragmaStatus != DistributedDB::DBStatus::OK) {
            ZLOGE("pragmaStatus: %d", static_cast<int>(pragmaStatus));
        }
    }

    status = RebuildKvStoreObserver(kvStoreNbDelegate);
    if (status != Status::SUCCESS) {
        ZLOGI("rebuild KvStore observer failed, errCode %d.", static_cast<int>(status));
        // skip this failed, continue to do other rebuild process.
    }

    status = RebuildKvStoreResultSet();
    if (status != Status::SUCCESS) {
        ZLOGI("rebuild KvStore resultset failed, errCode %d.", static_cast<int>(status));
        // skip this failed, continue to do close kvstore process.
    }

    ZLOGI("close old KvStore.");
    dbStatus = oldDelegateMgr->CloseKvStore(kvStoreNbDelegate_);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGI("rebuild KvStore failed during close KvStore, errCode %d.", static_cast<int>(status));
        newDelegateMgr->CloseKvStore(kvStoreNbDelegate);
        return Status::DB_ERROR;
    }

    ZLOGI("update kvstore delegate.");
    kvStoreNbDelegate_ = kvStoreNbDelegate;
    return Status::SUCCESS;
}

Status SingleKvStoreImpl::RebuildKvStoreObserver(DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate)
{
    ZLOGI("rebuild observer.");
    if (kvStoreNbDelegate_ == nullptr || kvStoreNbDelegate == nullptr) {
        ZLOGI("RebuildKvStoreObserver illlegal.");
        return Status::ILLEGAL_STATE;
    }
    std::lock_guard<std::mutex> observerMapLockGuard(observerMapMutex_);
    Status status = Status::SUCCESS;
    DistributedDB::DBStatus dbStatus;
    DistributedDB::Key emptyKey;
    for (const auto &observerPair : observerMap_) {
        dbStatus = kvStoreNbDelegate_->UnRegisterObserver(observerPair.second);
        if (dbStatus != DistributedDB::OK) {
            status = Status::DB_ERROR;
            ZLOGW("rebuild observer failed during UnRegisterObserver, status %d.", static_cast<int>(dbStatus));
            continue;
        }
        dbStatus = kvStoreNbDelegate->RegisterObserver(emptyKey,
            static_cast<unsigned int>(ConvertToDbObserverMode(observerPair.second->GetSubscribeType())),
            observerPair.second);
        if (dbStatus != DistributedDB::OK) {
            status = Status::DB_ERROR;
            ZLOGW("rebuild observer failed during RegisterObserver, status %d.", static_cast<int>(dbStatus));
            continue;
        }
    }
    return status;
}

Status SingleKvStoreImpl::RebuildKvStoreResultSet()
{
    if (kvStoreNbDelegate_ == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    ZLOGI("rebuild resultset");
    std::lock_guard<std::mutex> lg(storeResultSetMutex_);
    Status retStatus = Status::SUCCESS;
    for (const auto &resultSetPair : storeResultSetMap_) {
        Status status = (resultSetPair.first)->MigrateKvStore(kvStoreNbDelegate_);
        if (status != Status::SUCCESS) {
            retStatus = status;
            ZLOGW("rebuild resultset failed, errCode %d", static_cast<int>(status));
            continue;
        }
    }
    return retStatus;
}

Status SingleKvStoreImpl::ReKey(const std::vector<uint8_t> &key)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
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
        dbStatus = kvStoreNbDelegate_->Rekey(password);
    }
    if (dbStatus == DistributedDB::DBStatus::OK) {
        return Status::SUCCESS;
    }
    return Status::ERROR;
}

Status SingleKvStoreImpl::RegisterSyncCallback(sptr<IKvStoreSyncCallback> callback)
{
    syncCallback_ = std::move(callback);
    return Status::SUCCESS;
}

Status SingleKvStoreImpl::UnRegisterSyncCallback()
{
    syncCallback_ = nullptr;
    return Status::SUCCESS;
}

Status SingleKvStoreImpl::PutBatch(const std::vector<Entry> &entries)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
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
        status = kvStoreNbDelegate_->PutBatch(dbEntries);
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGE("PutBatch failed, distributeddb need recover.");
        return (Import(bundleName_) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED);
    }

    if (status == DistributedDB::DBStatus::EKEYREVOKED_ERROR ||
        status == DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR) {
        ZLOGE("delegate PutBatch failed.");
        return Status::SECURITY_LEVEL_ERROR;
    }

    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate PutBatch failed.");
        return Status::DB_ERROR;
    }

    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status SingleKvStoreImpl::DeleteBatch(const std::vector<Key> &keys)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
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
        status = kvStoreNbDelegate_->DeleteBatch(dbKeys);
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGE("DeleteBatch failed, distributeddb need recover.");
        return (Import(bundleName_) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED);
    }

    if (status == DistributedDB::DBStatus::EKEYREVOKED_ERROR ||
        status == DistributedDB::DBStatus::SECURITY_OPTION_CHECK_ERROR) {
        ZLOGE("delegate DeleteBatch failed.");
        return Status::SECURITY_LEVEL_ERROR;
    }

    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate DeleteBatch failed.");
        return Status::DB_ERROR;
    }

    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status SingleKvStoreImpl::StartTransaction()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->StartTransaction();
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGE("StartTransaction failed, distributeddb need recover.");
        return (Import(bundleName_) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED);
    }
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error.");
        return Status::DB_ERROR;
    }
    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status SingleKvStoreImpl::Commit()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->Commit();
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGE("Commit failed, distributeddb need recover.");
        return (Import(bundleName_) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED);
    }
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error.");
        return Status::DB_ERROR;
    }

    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status SingleKvStoreImpl::Rollback()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    if (kvStoreNbDelegate_ == nullptr) {
        ZLOGE("delegate is null.");
        return Status::DB_ERROR;
    }
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }
    DistributedDB::DBStatus status;
    {
        DdsTrace trace(std::string(LOG_TAG "Delegate::") + std::string(__FUNCTION__));
        status = kvStoreNbDelegate_->Rollback();
    }
    if (status == DistributedDB::DBStatus::INVALID_PASSWD_OR_CORRUPTED_DB) {
        ZLOGE("Rollback failed, distributeddb need recover.");
        return (Import(bundleName_) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED);
    }
    if (status != DistributedDB::DBStatus::OK) {
        ZLOGE("delegate return error.");
        return Status::DB_ERROR;
    }

    Reporter::GetInstance()->VisitStatistic()->Report({bundleName_, __FUNCTION__});
    return Status::SUCCESS;
}

Status SingleKvStoreImpl::Control(KvControlCmd cmd, const KvParam &inputParam, sptr<KvParam> &output)
{
    output = nullptr;
    switch (cmd) {
        case KvControlCmd::SET_SYNC_PARAM: {
            if (inputParam.Size() != sizeof(KvSyncParam)) {
                return Status::IPC_ERROR;
            }
            KvSyncParam syncParam = TransferByteArrayToType<KvSyncParam>(inputParam.Data());
            uint32_t allowedDelayMs = syncParam.allowedDelayMs;
            if (allowedDelayMs > 0 && allowedDelayMs < KvStoreSyncManager::SYNC_MIN_DELAY_MS) {
                return Status::INVALID_ARGUMENT;
            }
            if (allowedDelayMs > KvStoreSyncManager::SYNC_MAX_DELAY_MS) {
                return Status::INVALID_ARGUMENT;
            }
            defaultSyncDelayMs_ = allowedDelayMs;
            return Status::SUCCESS;
        }
        case KvControlCmd::GET_SYNC_PARAM: {
            KvSyncParam syncParam{defaultSyncDelayMs_};
            output = new KvParam(TransferTypeToByteArray<KvSyncParam>(syncParam));
            return Status::SUCCESS;
        }
        default: {
            ZLOGE("control invalid command.");
            return Status::ERROR;
        }
    }
}

void SingleKvStoreImpl::IncreaseOpenCount()
{
    openCount_++;
}

bool SingleKvStoreImpl::Import(const std::string &bundleName) const
{
    ZLOGI("Single KvStoreImpl Import start");
    const std::string harmonyAccountId = AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId();
    auto sKey = KvStoreMetaManager::GetMetaKey(deviceAccountId_, harmonyAccountId, bundleName, storeId_, "SINGLE_KEY");
    std::vector<uint8_t> secretKey;
    bool outdated = false;
    auto trueAppId = KvStoreUtils::GetAppIdByBundleName(bundleName);
    KvStoreMetaManager::GetInstance().GetSecretKeyFromMeta(sKey, secretKey, outdated);
    MetaData metaData{0};
    metaData.kvStoreMetaData.deviceAccountId = deviceAccountId_;
    metaData.kvStoreMetaData.userId = harmonyAccountId;
    metaData.kvStoreMetaData.bundleName = bundleName;
    metaData.kvStoreMetaData.appId = trueAppId;
    metaData.kvStoreMetaData.storeId = storeId_;
    metaData.secretKeyMetaData.secretKey = secretKey;
    std::shared_lock<std::shared_mutex> lock(storeNbDelegateMutex_);
    return std::make_unique<BackupHandler>()->SingleKvStoreRecover(metaData, kvStoreNbDelegate_);
}

Status SingleKvStoreImpl::SetCapabilityEnabled(bool enabled)
{
    ZLOGD("begin.");
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }

    std::string key;
    std::string devId = DeviceKvStoreImpl::GetLocalDeviceId();
    if (devId.empty()) {
        ZLOGE("get device id empty.");
        return Status::ERROR;
    }

    StrategyMeta params = {devId, deviceAccountId_, Constant::DEFAULT_GROUP_ID, bundleName_, storeId_};
    KvStoreMetaManager::GetInstance().GetStrategyMetaKey(params, key);
    if (key.empty()) {
        ZLOGE("get key empty.");
        return Status::ERROR;
    }
    ZLOGD("end.");
    return KvStoreMetaManager::GetInstance().SaveStrategyMetaEnable(key, enabled);
}

Status SingleKvStoreImpl::SetCapabilityRange(const std::vector<std::string> &localLabels,
                                             const std::vector<std::string> &remoteSupportLabels)
{
    if (!flowCtrlManager_.IsTokenEnough()) {
        ZLOGE("flow control denied");
        return Status::EXCEED_MAX_ACCESS_RATE;
    }

    std::string key;
    std::string devId = DeviceKvStoreImpl::GetLocalDeviceId();
    if (devId.empty()) {
        ZLOGE("get device id empty.");
        return Status::ERROR;
    }

    StrategyMeta params = {devId, deviceAccountId_, Constant::DEFAULT_GROUP_ID, bundleName_, storeId_};
    KvStoreMetaManager::GetInstance().GetStrategyMetaKey(params, key);
    if (key.empty()) {
        ZLOGE("get key empty.");
        return Status::ERROR;
    }

    return KvStoreMetaManager::GetInstance().SaveStrategyMetaLabels(key, localLabels, remoteSupportLabels);
}

Status SingleKvStoreImpl::GetSecurityLevel(SecurityLevel &securityLevel)
{
    if (kvStoreNbDelegate_ == nullptr) {
        return Status::STORE_NOT_OPEN;
    }

    DistributedDB::SecurityOption option;
    auto status = kvStoreNbDelegate_->GetSecurityOption(option);
    if (status == DistributedDB::DBStatus::NOT_SUPPORT) {
        return Status::NOT_SUPPORT;
    }

    if (status != DistributedDB::DBStatus::OK) {
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
    return Status::SUCCESS;
}

void SingleKvStoreImpl::OnDump(int fd) const
{
    const std::string prefix(12, ' ');
    dprintf(fd, "%s------------------------------------------------------\n", prefix.c_str());
    dprintf(fd, "%sStoreID    : %s\n", prefix.c_str(), storeId_.c_str());
    dprintf(fd, "%sStorePath  : %s\n", prefix.c_str(), storePath_.c_str());

    dprintf(fd, "%sOptions :\n", prefix.c_str());
    dprintf(fd, "%s    backup          : %d\n", prefix.c_str(), static_cast<int>(options_.backup));
    dprintf(fd, "%s    encrypt         : %d\n", prefix.c_str(), static_cast<int>(options_.encrypt));
    dprintf(fd, "%s    autoSync        : %d\n", prefix.c_str(), static_cast<int>(options_.autoSync));
    dprintf(fd, "%s    persistant      : %d\n", prefix.c_str(), static_cast<int>(options_.persistant));
    dprintf(fd, "%s    kvStoreType     : %d\n", prefix.c_str(), static_cast<int>(options_.kvStoreType));
    dprintf(fd, "%s    createIfMissing : %d\n", prefix.c_str(), static_cast<int>(options_.createIfMissing));
    dprintf(fd, "%s    schema          : %s\n", prefix.c_str(), options_.schema.c_str());
}
}  // namespace OHOS::DistributedKv
