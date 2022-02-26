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

#include "generic_kvdb.h"
#include "platform_specific.h"
#include "log_print.h"
#include "db_common.h"
#include "db_constant.h"
#include "db_errno.h"
#include "package_file.h"
#include "runtime_context.h"
#include "kvdb_utils.h"
#include "kvdb_commit_notify_filterable_data.h"

namespace DistributedDB {
DEFINE_OBJECT_TAG_FACILITIES(GenericKvDB);

GenericKvDB::GenericKvDB()
    : performance_(nullptr),
      eventNotifyCounter_(0),
      connectionCount_(0),
      notificationChain_(nullptr),
      operatePerm_(OperatePerm::NORMAL_PERM)
{}

GenericKvDB::~GenericKvDB()
{
    if (connectionCount_ > 0) {
        LOGF("KvDB destructed with connection count > 0.");
    }

    if (notificationChain_ != nullptr) {
        RefObject::KillAndDecObjRef(notificationChain_);
        notificationChain_ = nullptr;
    }
}

const KvDBProperties &GenericKvDB::GetMyProperties() const
{
    return MyProp();
}

IKvDBConnection *GenericKvDB::GetDBConnection(int &errCode)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (operatePerm_ != OperatePerm::NORMAL_PERM) {
        errCode = (operatePerm_ == OperatePerm::DISABLE_PERM) ? -E_STALE : -E_BUSY;
        return nullptr;
    }

    GenericKvDBConnection *connection = NewConnection(errCode);
    if (connection != nullptr) {
        IncObjRef(this);
        IncreaseConnectionCounter();
    }
    return connection;
}

void GenericKvDB::OnClose(const std::function<void(void)> &notifier)
{
    AutoLock lockGuard(this);
    if (notifier) {
        closeNotifiers_.push_back(notifier);
    } else {
        LOGW("Register kvdb 'Close()' notifier failed, notifier is null.");
    }
}

std::string GenericKvDB::GetStoreId() const
{
    return MyProp().GetStringProp(KvDBProperties::STORE_ID, "");
}

void GenericKvDB::DelConnection(GenericKvDBConnection *connection)
{
    delete connection;
    connection = nullptr;
}

void GenericKvDB::ReleaseDBConnection(GenericKvDBConnection *connection)
{
    if (connectionCount_.load() == 1) {
        SetConnectionFlag(false);
    }

    connectMutex_.lock();
    if (connection != nullptr) {
        connection->SetSafeDeleted();
        DelConnection(connection);
        DecreaseConnectionCounter();
        connectMutex_.unlock();
        DecObjRef(this);
    } else {
        connectMutex_.unlock();
    }
}

void GenericKvDB::CommitNotify(int notifyEvent, KvDBCommitNotifyFilterAbleData *data)
{
    if (notificationChain_ == nullptr) {
        LOGE("Failed to do commit notify, notificationChain_ is nullptr.");
        return;
    }
    ++eventNotifyCounter_;
    if (data == nullptr) {
        notificationChain_->NotifyEvent(static_cast<EventType>(notifyEvent), nullptr);
    } else {
        data->SetMyDb(this, eventNotifyCounter_);
        data->IncObjRef(data);
        int errCode = RuntimeContext::GetInstance()->ScheduleQueuedTask(GetStoreId(),
            std::bind(&GenericKvDB::CommitNotifyAsync, this, notifyEvent, data));
        if (errCode != E_OK) {
            LOGE("Failed to do commit notify, schedule task err:%d.", errCode);
            data->DecObjRef(data);
            data = nullptr;
        }
    }
}

void GenericKvDB::CorruptNotifyAsync() const
{
    {
        std::lock_guard<std::mutex> lock(corruptMutex_);
        if (corruptHandler_) {
            corruptHandler_();
        }
    }

    DecObjRef(this);
}

void GenericKvDB::CorruptNotify() const
{
    IncObjRef(this);
    int errCode = RuntimeContext::GetInstance()->ScheduleQueuedTask(GetStoreId(),
        std::bind(&GenericKvDB::CorruptNotifyAsync, this));
    if (errCode != E_OK) {
        LOGE("Failed to do the corrupt notify, schedule task err:%d.", errCode);
        DecObjRef(this);
    }
}

int GenericKvDB::TryToDisableConnection(OperatePerm perm)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (operatePerm_ != OperatePerm::NORMAL_PERM) {
        return -E_BUSY;
    }
    // more than one connection, should prevent the rekey operation.
    if (connectionCount_ > 1) {
        return -E_BUSY;
    }

    operatePerm_ = perm;
    return E_OK;
}

void GenericKvDB::ReEnableConnection(OperatePerm perm)
{
    std::lock_guard<std::mutex> lock(connectMutex_);
    if (perm == operatePerm_) {
        operatePerm_ = OperatePerm::NORMAL_PERM;
    }
}

NotificationChain::Listener *GenericKvDB::RegisterEventListener(EventType type,
    const NotificationChain::Listener::OnEvent &onEvent,
    const NotificationChain::Listener::OnFinalize &onFinalize, int &errCode)
{
    NotificationChain::Listener *listener = nullptr;
    if (notificationChain_ != nullptr) {
        listener = notificationChain_->RegisterListener(type, onEvent, onFinalize, errCode);
    } else {
        errCode = -E_NOT_PERMIT;
    }
    return listener;
}

uint64_t GenericKvDB::GetEventNotifyCounter() const
{
    return eventNotifyCounter_;
}

// Called when a new connection created.
void GenericKvDB::IncreaseConnectionCounter()
{
    connectionCount_.fetch_add(1, std::memory_order_seq_cst);
    if (connectionCount_.load() > 0) {
        SetConnectionFlag(true);
    }
}

// Called when a connection released.
void GenericKvDB::DecreaseConnectionCounter()
{
    int count = connectionCount_.fetch_sub(1, std::memory_order_seq_cst);
    if (count <= 0) {
        LOGF("Decrease kvdb connection counter failed, count <= 0.");
        return;
    }
    if (count != 1) {
        return;
    }

    operatePerm_ = OperatePerm::DISABLE_PERM;
    LockObj();
    auto notifiers = std::move(closeNotifiers_);
    UnlockObj();

    for (auto &notifier : notifiers) {
        if (notifier) {
            notifier();
        }
    }

    Close();
}

// Register a new notification event type.
int GenericKvDB::RegisterNotificationEventType(int eventType)
{
    if (notificationChain_ == nullptr) {
        // Lazy init.
        notificationChain_ = new (std::nothrow) NotificationChain;
        if (notificationChain_ == nullptr) {
            return -E_OUT_OF_MEMORY;
        }
    }
    // We DON'T release 'notificationChain_' here event if RegisterEventType()
    // is failed, it belongs to the class object and is released in the destructor.
    return notificationChain_->RegisterEventType(static_cast<EventType>(eventType));
}

// Unregister a notification event type.
void GenericKvDB::UnRegisterNotificationEventType(int eventType)
{
    if (notificationChain_ != nullptr) {
        notificationChain_->UnRegisterEventType(static_cast<EventType>(eventType));
    }
}

const KvDBProperties &GenericKvDB::MyProp() const
{
    return properties_;
}

KvDBProperties &GenericKvDB::MyProp()
{
    return properties_;
}

int GenericKvDB::GetWorkDir(const KvDBProperties &kvDBProp, std::string &workDir)
{
    std::string origDataDir = kvDBProp.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifierDir = kvDBProp.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    if (origDataDir.empty()) {
        return -E_INVALID_ARGS;
    }

    workDir = origDataDir + "/" + identifierDir;
    return E_OK;
}

void GenericKvDB::SetCorruptHandler(const DatabaseCorruptHandler &handler)
{
    std::lock_guard<std::mutex> lock(corruptMutex_);
    corruptHandler_ = handler;
}

void GenericKvDB::OpenPerformanceAnalysis()
{
    if (performance_ != nullptr) {
        performance_->OpenPerformanceAnalysis();
    }
}

void GenericKvDB::ClosePerformanceAnalysis()
{
    if (performance_ != nullptr) {
        performance_->ClosePerformanceAnalysis();
    }
}

void GenericKvDB::CommitNotifyAsync(int notifyEvent, KvDBCommitNotifyFilterAbleData *data)
{
    notificationChain_->NotifyEvent(static_cast<EventType>(notifyEvent), data);
    data->DecObjRef(data);
    data = nullptr;
}

int GenericKvDB::RegisterFunction(RegisterFuncType type)
{
    if (type >= REGISTER_FUNC_TYPE_MAX) {
        return -E_NOT_SUPPORT;
    }
    std::lock_guard<std::mutex> lock(regFuncCountMutex_);
    if (registerFunctionCount_.empty()) {
        registerFunctionCount_.resize(static_cast<uint32_t>(REGISTER_FUNC_TYPE_MAX), 0);
        if (registerFunctionCount_.size() != static_cast<size_t>(REGISTER_FUNC_TYPE_MAX)) {
            return -E_OUT_OF_MEMORY;
        }
    }
    registerFunctionCount_[type]++;
    return E_OK;
}

int GenericKvDB::UnregisterFunction(RegisterFuncType type)
{
    if (type >= REGISTER_FUNC_TYPE_MAX) {
        return -E_NOT_SUPPORT;
    }
    std::lock_guard<std::mutex> lock(regFuncCountMutex_);
    if (registerFunctionCount_.size() != static_cast<size_t>(REGISTER_FUNC_TYPE_MAX) ||
        registerFunctionCount_[type] == 0) {
        return -E_UNEXPECTED_DATA;
    }
    registerFunctionCount_[type]--;
    return E_OK;
}

uint32_t GenericKvDB::GetRegisterFunctionCount(RegisterFuncType type) const
{
    std::lock_guard<std::mutex> lock(regFuncCountMutex_);
    if (type >= REGISTER_FUNC_TYPE_MAX ||
        registerFunctionCount_.size() != static_cast<size_t>(REGISTER_FUNC_TYPE_MAX)) {
        return 0;
    }
    return registerFunctionCount_[type];
}

int GenericKvDB::TransObserverTypeToRegisterFunctionType(int observerType, RegisterFuncType &type) const
{
    return -E_NOT_SUPPORT;
}

int GenericKvDB::TransConflictTypeToRegisterFunctionType(int conflictType, RegisterFuncType &type) const
{
    return -E_NOT_SUPPORT;
}

int GenericKvDB::CheckDataStatus(const Key &key, const Value &value, bool isDeleted) const
{
    if (key.empty() || key.size() > DBConstant::MAX_KEY_SIZE ||
        value.size() > DBConstant::MAX_VALUE_SIZE) {
        return -E_INVALID_ARGS;
    }
    return E_OK;
}

bool GenericKvDB::CheckWritePermission() const
{
    return true;
}

bool GenericKvDB::IsDataMigrating() const
{
    return false;
}

void GenericKvDB::SetConnectionFlag(bool isExisted) const
{
    return;
}

void GenericKvDB::GetStoreDirectory(const KvDBProperties &properties, int dbType,
    std::string &storeDir, std::string &storeOnlyDir) const
{
    std::string identifierDir = properties.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    std::string dataDir = properties.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string subDir = KvDBProperties::GetStoreSubDirectory(dbType);

    std::string storeOnlyIdentifier = GetStoreIdOnlyIdentifier(properties);
    storeOnlyDir = dataDir + "/" + storeOnlyIdentifier + "/" + subDir + "/";
    storeDir = dataDir + "/" + identifierDir + "/" + subDir + "/";
}

std::string GenericKvDB::GetStoreIdOnlyIdentifier(const KvDBProperties &properties) const
{
    std::string storeId = properties.GetStringProp(KvDBProperties::STORE_ID, "");
    std::string hashStoreId = DBCommon::TransferHashString(storeId);
    std::string hashStoreDir = DBCommon::TransferStringToHex(hashStoreId);
    return hashStoreDir;
}
} // namespace DistributedDB
