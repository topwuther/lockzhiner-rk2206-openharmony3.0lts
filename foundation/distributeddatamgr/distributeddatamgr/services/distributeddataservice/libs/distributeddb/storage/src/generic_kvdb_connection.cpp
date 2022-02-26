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

#include "generic_kvdb_connection.h"

#include <algorithm>

#include "log_print.h"
#include "db_constant.h"
#include "db_errno.h"
#include "generic_kvdb.h"
#include "kvdb_observer_handle.h"
#include "kvdb_commit_notify_filterable_data.h"

namespace DistributedDB {
GenericKvDBConnection::GenericKvDBConnection(GenericKvDB *kvDB)
    : kvDB_(kvDB),
      isExclusive_(false),
      isSafeDeleted_(false)
{
}

GenericKvDBConnection::~GenericKvDBConnection()
{
    if (!isSafeDeleted_) {
        LOGF("The connection is deleted directly by user.");
    }

    for (auto &observer : observerList_) {
        delete observer;
        observer = nullptr;
    }
}

int GenericKvDBConnection::RegisterObserverForOneType(int type, const Key &key, const KvDBObserverAction &action,
    NotificationChain::Listener *&listener)
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    RegisterFuncType funcType = REGISTER_FUNC_TYPE_MAX;
    int errCode = kvDB_->TransObserverTypeToRegisterFunctionType(type, funcType);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = kvDB_->RegisterFunction(funcType);
    if (errCode != E_OK) {
        return errCode;
    }
    listener = RegisterSpecialListener(type, key, action, false, errCode);
    if (listener == nullptr) {
        (void)(kvDB_->UnregisterFunction(funcType));
        return errCode;
    }
    return E_OK;
}

KvDBObserverHandle *GenericKvDBConnection::RegisterObserver(unsigned mode,
    const Key &key, const KvDBObserverAction &action, int &errCode)
{
    if (!action || key.size() > DBConstant::MAX_KEY_SIZE) {
        errCode = -E_INVALID_ARGS;
        return nullptr;
    }
    std::list<int> eventTypes;
    errCode = GetEventType(mode, eventTypes);
    if (errCode != E_OK) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lockGuard(observerListLock_);
    if (observerList_.size() >= MAX_OBSERVER_COUNT) {
        errCode = -E_MAX_LIMITS;
        LOGE("The number of observers has been larger than 'MAX_OBSERVER_COUNT'!");
        return nullptr;
    }
    if (isExclusive_.load()) {
        errCode = -E_BUSY;
        return nullptr;
    }
    auto observerHandle = new (std::nothrow) KvDBObserverHandle(mode);
    if (observerHandle == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }

    std::list<NotificationChain::Listener *> listenerList;
    for (const auto &type : eventTypes) {
        NotificationChain::Listener *listenerObj = nullptr;
        // Register function count in db is also protected by observer list lock.
        errCode = RegisterObserverForOneType(type, key, action, listenerObj);
        if (errCode != E_OK) {
            for (auto &listener : listenerList) {
                listener->Drop();
            }
            LOGE("Register observer failed, register listener failed, err:'%d'.", errCode);
            delete observerHandle;
            observerHandle = nullptr;
            return nullptr;
        }
        listenerList.push_back(listenerObj);
    }

    for (auto &listener : listenerList) {
        observerHandle->InsertListener(listener);
    }
    observerList_.push_back(observerHandle);
    errCode = E_OK;
    return observerHandle;
}

int GenericKvDBConnection::UnRegisterObserver(const KvDBObserverHandle *observerHandle)
{
    if (observerHandle == nullptr) {
        return -E_INVALID_ARGS;
    }

    if (kvDB_ == nullptr) {
        return -E_INVALID_CONNECTION;
    }

    std::list<int> eventTypes;
    int errCode = GetEventType(observerHandle->GetObserverMode(), eventTypes);
    if (errCode != E_OK) {
        return errCode;
    }

    {
        std::lock_guard<std::mutex> lockGuard(observerListLock_);
        auto observerIter = std::find(observerList_.begin(), observerList_.end(), observerHandle);
        if (observerIter == observerList_.end()) {
            LOGE("Unregister observer failed, no such entry.");
            return -E_NO_SUCH_ENTRY;
        }
        observerList_.erase(observerIter);
        // Register function count in db is also protected by observer list lock.
        RegisterFuncType funcType = REGISTER_FUNC_TYPE_MAX;
        for (auto type : eventTypes) {
            errCode = kvDB_->TransObserverTypeToRegisterFunctionType(type, funcType);
            if (errCode != E_OK) {
                LOGE("Get register function type failed, err:'%d'.", errCode);
                continue;
            }
            errCode = kvDB_->UnregisterFunction(funcType);
            if (errCode != E_OK) {
                LOGE("Unregister function failed, err:'%d'.", errCode);
                continue;
            }
        }
    }

    delete observerHandle;
    observerHandle = nullptr;
    return E_OK;
}

int GenericKvDBConnection::SetConflictNotifier(int conflictType, const KvDBConflictAction &action)
{
    return -E_NOT_SUPPORT;
}

int GenericKvDBConnection::Close()
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_CONNECTION;
    }

    if (isExclusive_.load()) {
        return -E_BUSY;
    }
    if (kvDB_->IsDataMigrating()) {
        return -E_BUSY;
    }

    int errCode = PreClose();
    if (errCode != E_OK) {
        LOGE("Close connection  failed, err:'%d'.", errCode);
        return errCode;
    }
    kvDB_->ReleaseDBConnection(this);
    return E_OK;
}

std::string GenericKvDBConnection::GetIdentifier() const
{
    if (kvDB_ == nullptr) {
        return "";
    }
    return kvDB_->GetMyProperties().GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
}

int GenericKvDBConnection::Pragma(int cmd, void *parameter)
{
    return -E_NOT_SUPPORT;
}

int GenericKvDBConnection::PreClose()
{
    return E_OK;
}

void GenericKvDBConnection::SetSafeDeleted()
{
    isSafeDeleted_ = true;
}

int GenericKvDBConnection::GetEntries(const IOption &option, const Key &keyPrefix, std::vector<Entry> &entries) const
{
    return -E_NOT_SUPPORT;
}

int GenericKvDBConnection::GetEntries(const IOption &option, const Query &query, std::vector<Entry> &entries) const
{
    return -E_NOT_SUPPORT;
}

int GenericKvDBConnection::GetResultSet(const IOption &option, const Key &keyPrefix,
    IKvDBResultSet *&resultSet) const
{
    return -E_NOT_SUPPORT;
}

int GenericKvDBConnection::GetResultSet(const IOption &option, const Query &query, IKvDBResultSet *&resultSet) const
{
    return -E_NOT_SUPPORT;
}

int GenericKvDBConnection::GetCount(const IOption &option, const Query &query, int &count) const
{
    return -E_NOT_SUPPORT;
}

void GenericKvDBConnection::ReleaseResultSet(IKvDBResultSet *&resultSet)
{
    return;
}

int GenericKvDBConnection::RegisterLifeCycleCallback(const DatabaseLifeCycleNotifier &notifier)
{
    return -E_NOT_SUPPORT;
}

int GenericKvDBConnection::GetSecurityOption(int &securityLabel, int &securityFlag) const
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    securityLabel = kvDB_->GetMyProperties().GetIntProp(KvDBProperties::SECURITY_LABEL, 0);
    securityFlag = kvDB_->GetMyProperties().GetIntProp(KvDBProperties::SECURITY_FLAG, 0);
    return E_OK;
}

NotificationChain::Listener *GenericKvDBConnection::RegisterSpecialListener(int type,
    const Key &key, const KvDBObserverAction &action, bool conflict, int &errCode)
{
    if (!action) {
        errCode = -E_INVALID_ARGS;
        return nullptr;
    }

    if (kvDB_ == nullptr) {
        errCode = -E_INVALID_CONNECTION;
        return nullptr;
    }

    uint64_t notifyBarrier = kvDB_->GetEventNotifyCounter();
    return kvDB_->RegisterEventListener(static_cast<EventType>(type),
        [key, action, conflict, notifyBarrier](void *ptr) {
            if (ptr == nullptr) {
                return;
            }
            KvDBCommitNotifyFilterAbleData *data = static_cast<KvDBCommitNotifyFilterAbleData *>(ptr);
            if (data->GetNotifyID() <= notifyBarrier) {
                return;
            }
            data->SetFilterKey(key);
            if (conflict) {
                if (!data->IsConflictedDataEmpty()) {
                    action(*data);
                }
            } else {
                if (!data->IsChangedDataEmpty()) {
                    action(*data);
                }
            }
        }, nullptr, errCode);
}

int GenericKvDBConnection::PreCheckExclusiveStatus()
{
    std::lock_guard<std::mutex> lockGuard(observerListLock_);
    if (observerList_.empty()) {
        isExclusive_.store(true);
        return E_OK;
    }
    return -E_BUSY;
}

void GenericKvDBConnection::ResetExclusiveStatus()
{
    isExclusive_.store(false);
}

int GenericKvDBConnection::GetEventType(unsigned mode, std::list<int> &eventTypes) const
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_CONNECTION;
    }

    return TranslateObserverModeToEventTypes(mode, eventTypes);
}
}

