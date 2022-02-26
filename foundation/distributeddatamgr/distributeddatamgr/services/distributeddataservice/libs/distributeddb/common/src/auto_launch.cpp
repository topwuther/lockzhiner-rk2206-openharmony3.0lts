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

#include "auto_launch.h"

#include <map>

#include "db_errno.h"
#include "log_print.h"
#include "runtime_context.h"
#include "kvdb_pragma.h"
#include "kvdb_manager.h"
#include "kv_store_changed_data_impl.h"
#include "sync_able_kvdb_connection.h"
#include "semaphore.h"
#include "kv_store_nb_conflict_data_impl.h"
#include "db_common.h"
#include "param_check_utils.h"

namespace DistributedDB {
namespace {
    constexpr int MAX_AUTO_LAUNCH_ITEM_NUM = 8;
}

void AutoLaunch::SetCommunicatorAggregator(ICommunicatorAggregator *aggregator)
{
    LOGI("[AutoLaunch] SetCommunicatorAggregator");
    std::lock_guard<std::mutex> autoLock(communicatorLock_);
    int errCode;
    if (communicatorAggregator_ != nullptr) {
        LOGI("[AutoLaunch] SetCommunicatorAggregator communicatorAggregator_ is not nullptr");
        errCode = communicatorAggregator_->RegOnConnectCallback(nullptr, nullptr);
        if (errCode != E_OK) {
            LOGW("[AutoLaunch] communicatorAggregator_->RegOnConnectCallback(nullptr, nullptr), errCode:%d", errCode);
        }
        errCode = communicatorAggregator_->RegCommunicatorLackCallback(nullptr, nullptr);
        if (errCode != E_OK) {
            LOGW("[AutoLaunch] communicatorAggregator_->RegCommunicatorLackCallback(nullptr, nullptr), errCode:%d",
                errCode);
        }
    }
    communicatorAggregator_ = aggregator;
    if (aggregator == nullptr) {
        LOGI("[AutoLaunch] SetCommunicatorAggregator aggregator is nullptr");
        return;
    }
    errCode = aggregator->RegOnConnectCallback(std::bind(&AutoLaunch::OnlineCallBack, this,
        std::placeholders::_1, std::placeholders::_2), nullptr);
    if (errCode != E_OK) {
        LOGW("[AutoLaunch] aggregator->RegOnConnectCallback errCode:%d", errCode);
    }
    errCode = aggregator->RegCommunicatorLackCallback(
        std::bind(&AutoLaunch::ReceiveUnknownIdentifierCallBack, this, std::placeholders::_1), nullptr);
    if (errCode != E_OK) {
        LOGW("[AutoLaunch] aggregator->RegCommunicatorLackCallback errCode:%d", errCode);
    }
}

AutoLaunch::~AutoLaunch()
{
    {
        std::lock_guard<std::mutex> autoLock(communicatorLock_);
        LOGI("[AutoLaunch] ~AutoLaunch()");
        if (communicatorAggregator_ != nullptr) {
            communicatorAggregator_->RegOnConnectCallback(nullptr, nullptr);
            communicatorAggregator_->RegCommunicatorLackCallback(nullptr, nullptr);
            communicatorAggregator_ = nullptr;
        }
    }

    std::set<std::string> inDisableSet;
    std::set<std::string> inWaitIdleSet;
    std::unique_lock<std::mutex> autoLock(dataLock_);
    for (auto &iter : autoLaunchItemMap_) {
        if (iter.second.isDisable) {
            inDisableSet.insert(iter.first);
        } else if (iter.second.state == AutoLaunchItemState::IDLE && (!iter.second.inObserver)) {
            TryCloseConnection(iter.second);
        } else {
            inWaitIdleSet.insert(iter.first);
            iter.second.isDisable = true;
        }
    }
    for (const auto &identifier : inDisableSet) {
        cv_.wait(autoLock, [identifier, this] {
            return autoLaunchItemMap_.count(identifier) == 0 || (!autoLaunchItemMap_[identifier].isDisable);
        });
        if (autoLaunchItemMap_.count(identifier) != 0) {
            TryCloseConnection(autoLaunchItemMap_[identifier]);
        }
    }
    for (const auto &identifier : inWaitIdleSet) {
        cv_.wait(autoLock, [identifier, this] {
            return (autoLaunchItemMap_[identifier].state == AutoLaunchItemState::IDLE) &&
                (!autoLaunchItemMap_[identifier].inObserver);
        });
        TryCloseConnection(autoLaunchItemMap_[identifier]);
    }
}

int AutoLaunch::EnableKvStoreAutoLaunchParmCheck(AutoLaunchItem &autoLaunchItem, const std::string &identifier)
{
    if (identifier.empty()) {
        LOGE("[AutoLaunch] EnableKvStoreAutoLaunchParmCheck identifier is invalid");
        return -E_INVALID_ARGS;
    }
    std::lock_guard<std::mutex> autoLock(dataLock_);
    if (autoLaunchItemMap_.count(identifier) != 0) {
        LOGE("[AutoLaunch] EnableKvStoreAutoLaunchParmCheck identifier is already enabled!");
        return -E_ALREADY_SET;
    }
    if (autoLaunchItemMap_.size() == MAX_AUTO_LAUNCH_ITEM_NUM) {
        LOGE("[AutoLaunch] EnableKvStoreAutoLaunchParmCheck size is max(8) now");
        return -E_MAX_LIMITS;
    }
    autoLaunchItem.state = AutoLaunchItemState::IN_ENABLE;
    autoLaunchItemMap_[identifier] = autoLaunchItem;
    LOGI("[AutoLaunch] EnableKvStoreAutoLaunchParmCheck insert map first");
    return E_OK;
}

int AutoLaunch::EnableKvStoreAutoLaunch(const KvDBProperties &properties, AutoLaunchNotifier notifier,
    KvStoreObserver *observer, int conflictType, KvStoreNbConflictNotifier conflictNotifier)
{
    LOGI("[AutoLaunch] EnableKvStoreAutoLaunch");
    std::string identifier = properties.GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    AutoLaunchItem autoLaunchItem{properties, notifier, observer, conflictType, conflictNotifier};
    int errCode = EnableKvStoreAutoLaunchParmCheck(autoLaunchItem, identifier);
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] EnableKvStoreAutoLaunch failed errCode:%d", errCode);
        return errCode;
    }
    errCode = GetConnectionInEnable(autoLaunchItem, identifier);
    if (errCode == E_OK) {
        LOGI("[AutoLaunch] EnableKvStoreAutoLaunch ok");
    } else {
        LOGE("[AutoLaunch] EnableKvStoreAutoLaunch failed errCode:%d", errCode);
    }
    return errCode;
}

int AutoLaunch::GetConnectionInEnable(AutoLaunchItem &autoLaunchItem, const std::string &identifier)
{
    LOGI("[AutoLaunch] GetConnectionInEnable");
    int errCode;
    autoLaunchItem.conn = KvDBManager::GetDatabaseConnection(autoLaunchItem.properties, errCode, false);
    if (errCode == -E_ALREADY_OPENED) {
        LOGI("[AutoLaunch] GetConnectionInEnable user already getkvstore by self");
        std::lock_guard<std::mutex> autoLock(dataLock_);
        autoLaunchItemMap_[identifier].state = AutoLaunchItemState::IDLE;
        return E_OK;
    }
    if (autoLaunchItem.conn == nullptr) {
        LOGE("[AutoLaunch] GetConnectionInEnable GetDatabaseConnection errCode:%d", errCode);
        std::lock_guard<std::mutex> autoLock(dataLock_);
        autoLaunchItemMap_.erase(identifier);
        return errCode;
    }
    if (onlineDevices_.empty()) {
        LOGI("[AutoLaunch] GetConnectionInEnable no online device, ReleaseDatabaseConnection");
        errCode = KvDBManager::ReleaseDatabaseConnection(autoLaunchItem.conn);
        if (errCode != E_OK) {
            LOGE("[AutoLaunch] GetConnectionInEnable ReleaseDatabaseConnection failed errCode:%d", errCode);
            std::lock_guard<std::mutex> autoLock(dataLock_);
            autoLaunchItemMap_.erase(identifier);
            return errCode;
        }
        std::lock_guard<std::mutex> autoLock(dataLock_);
        autoLaunchItemMap_[identifier].state = AutoLaunchItemState::IDLE;
        return E_OK;
    }
    errCode = RegisterObserverAndLifeCycleCallback(autoLaunchItem, identifier, false);
    if (errCode == E_OK) {
        std::lock_guard<std::mutex> autoLock(dataLock_);
        autoLaunchItemMap_[identifier].state = AutoLaunchItemState::IDLE;
        autoLaunchItemMap_[identifier].conn = autoLaunchItem.conn;
        autoLaunchItemMap_[identifier].observerHandle = autoLaunchItem.observerHandle;
        LOGI("[AutoLaunch] GetConnectionInEnable RegisterObserverAndLifeCycleCallback ok");
    } else {
        LOGE("[AutoLaunch] GetConnectionInEnable RegisterObserverAndLifeCycleCallback err, do CloseConnection");
        TryCloseConnection(autoLaunchItem); // do nothing if failed
        std::lock_guard<std::mutex> autoLock(dataLock_);
        autoLaunchItemMap_.erase(identifier);
    }
    return errCode;
}

// we will return errCode, if errCode != E_OK
int AutoLaunch::CloseConnectionStrict(AutoLaunchItem &autoLaunchItem)
{
    LOGI("[AutoLaunch] CloseConnectionStrict");
    if (autoLaunchItem.conn == nullptr) {
        LOGI("[AutoLaunch] CloseConnectionStrict conn is nullptr, do nothing");
        return E_OK;
    }
    int errCode = autoLaunchItem.conn->RegisterLifeCycleCallback(nullptr);
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] CloseConnectionStrict RegisterLifeCycleCallback failed errCode:%d", errCode);
        return errCode;
    }
    if (autoLaunchItem.observerHandle != nullptr) {
        errCode = autoLaunchItem.conn->UnRegisterObserver(autoLaunchItem.observerHandle);
        if (errCode != E_OK) {
            LOGE("[AutoLaunch] CloseConnectionStrict UnRegisterObserver failed errCode:%d", errCode);
            return errCode;
        }
        autoLaunchItem.observerHandle = nullptr;
    }
    errCode = KvDBManager::ReleaseDatabaseConnection(autoLaunchItem.conn);
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] CloseConnectionStrict ReleaseDatabaseConnection failed errCode:%d", errCode);
    }
    return errCode;
}

// before ReleaseDatabaseConnection, if errCode != E_OK, we not return, we try close more
void AutoLaunch::TryCloseConnection(AutoLaunchItem &autoLaunchItem)
{
    LOGI("[AutoLaunch] TryCloseConnection");
    if (autoLaunchItem.conn == nullptr) {
        LOGI("[AutoLaunch] TryCloseConnection conn is nullptr, do nothing");
        return;
    }
    int errCode = autoLaunchItem.conn->RegisterLifeCycleCallback(nullptr);
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] TryCloseConnection RegisterLifeCycleCallback failed errCode:%d", errCode);
    }
    if (autoLaunchItem.observerHandle != nullptr) {
        errCode = autoLaunchItem.conn->UnRegisterObserver(autoLaunchItem.observerHandle);
        if (errCode != E_OK) {
            LOGE("[AutoLaunch] TryCloseConnection UnRegisterObserver failed errCode:%d", errCode);
        }
        autoLaunchItem.observerHandle = nullptr;
    }
    errCode = KvDBManager::ReleaseDatabaseConnection(autoLaunchItem.conn);
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] TryCloseConnection ReleaseDatabaseConnection failed errCode:%d", errCode);
    }
}

int AutoLaunch::RegisterObserverAndLifeCycleCallback(AutoLaunchItem &autoLaunchItem, const std::string &identifier,
    bool isExt)
{
    int errCode = RegisterObserver(autoLaunchItem, identifier, isExt);
    if (errCode != E_OK) {
        return errCode;
    }
    LOGI("[AutoLaunch] RegisterObserver ok");
    if (isExt) {
        errCode = autoLaunchItem.conn->RegisterLifeCycleCallback(std::bind(
            &AutoLaunch::ExtConnectionLifeCycleCallback, this, std::placeholders::_1));
    } else {
        errCode = autoLaunchItem.conn->RegisterLifeCycleCallback(std::bind(&AutoLaunch::ConnectionLifeCycleCallback,
            this, std::placeholders::_1));
    }
    if (errCode != E_OK) {
        LOGE("[AutoLaunch]  RegisterLifeCycleCallback failed, errCode:%d", errCode);
        return errCode;
    }
    LOGI("[AutoLaunch] RegisterLifeCycleCallback ok");

    errCode = SetConflictNotifier(autoLaunchItem.conn, autoLaunchItem.conflictType, autoLaunchItem.conflictNotifier);
    if (errCode != E_OK) {
        LOGE("[AutoLaunch]  SetConflictNotifier failed, errCode:%d", errCode);
        return errCode;
    }

    bool enAutoSync = true;
    errCode = static_cast<SyncAbleKvDBConnection *>(autoLaunchItem.conn)->Pragma(PRAGMA_AUTO_SYNC,
        static_cast<void *>(&enAutoSync));
    if (errCode != E_OK) {
        LOGE("[AutoLaunch]  PRAGMA_AUTO_SYNC failed, errCode:%d", errCode);
        return errCode;
    }
    LOGI("[AutoLaunch] PRAGMA_AUTO_SYNC ok");
    return errCode;
}

int AutoLaunch::RegisterObserver(AutoLaunchItem &autoLaunchItem, const std::string &identifier, bool isExt)
{
    LOGI("[AutoLaunch] RegisterObserver");
    if (autoLaunchItem.conn == nullptr) {
        LOGE("[AutoLaunch] autoLaunchItem.conn is nullptr");
        return -E_INTERNAL_ERROR;
    }
    int errCode;
    Key key;
    KvDBObserverHandle *observerHandle = nullptr;
    if (isExt) {
        observerHandle = autoLaunchItem.conn->RegisterObserver(OBSERVER_CHANGES_FOREIGN, key,
            std::bind(&AutoLaunch::ExtObserverFunc, this, std::placeholders::_1, identifier), errCode);
    } else {
        observerHandle = autoLaunchItem.conn->RegisterObserver(OBSERVER_CHANGES_FOREIGN, key,
            std::bind(&AutoLaunch::ObserverFunc, this, std::placeholders::_1, identifier), errCode);
    }

    if (errCode != E_OK) {
        LOGE("[AutoLaunch] RegisterObserver failed:%d!", errCode);
        return errCode;
    }
    autoLaunchItem.observerHandle = observerHandle;
    return E_OK;
}

void AutoLaunch::ObserverFunc(const KvDBCommitNotifyData &notifyData, const std::string &identifier)
{
    LOGD("[AutoLaunch] ObserverFunc");
    AutoLaunchItem autoLaunchItem;
    std::string userId;
    std::string appId;
    std::string storeId;
    {
        std::lock_guard<std::mutex> autoLock(dataLock_);
        if (autoLaunchItemMap_.count(identifier) == 0) {
            LOGE("[AutoLaunch] ObserverFunc err no this identifier in map");
            return;
        }
        if (autoLaunchItemMap_[identifier].isDisable) {
            LOGI("[AutoLaunch] ObserverFunc isDisable, do nothing");
            return;
        }
        autoLaunchItemMap_[identifier].inObserver = true;
        autoLaunchItem.observer = autoLaunchItemMap_[identifier].observer;
        autoLaunchItem.isWriteOpenNotifiered = autoLaunchItemMap_[identifier].isWriteOpenNotifiered;
        autoLaunchItem.notifier = autoLaunchItemMap_[identifier].notifier;
        userId = autoLaunchItemMap_[identifier].properties.GetStringProp(KvDBProperties::USER_ID, "");
        appId = autoLaunchItemMap_[identifier].properties.GetStringProp(KvDBProperties::APP_ID, "");
        storeId = autoLaunchItemMap_[identifier].properties.GetStringProp(KvDBProperties::STORE_ID, "");
    }
    if (autoLaunchItem.observer != nullptr) {
        LOGI("[AutoLaunch] do user observer");
        KvStoreChangedDataImpl data(&notifyData);
        (autoLaunchItem.observer)->OnChange(data);
    }
    LOGI("[AutoLaunch] in observer autoLaunchItem.isWriteOpenNotifiered:%d", autoLaunchItem.isWriteOpenNotifiered);

    if (!autoLaunchItem.isWriteOpenNotifiered && autoLaunchItem.notifier != nullptr) {
        {
            std::lock_guard<std::mutex> autoLock(dataLock_);
            autoLaunchItemMap_[identifier].isWriteOpenNotifiered = true;
        }
        AutoLaunchNotifier notifier = autoLaunchItem.notifier;
        int retCode = RuntimeContext::GetInstance()->ScheduleTask([notifier, userId, appId, storeId] {
            LOGI("[AutoLaunch] notify the user auto opened event");
            notifier(userId, appId, storeId, AutoLaunchStatus::WRITE_OPENED);
        });
        if (retCode != E_OK) {
            LOGE("[AutoLaunch] ObserverFunc notifier ScheduleTask retCode:%d", retCode);
        }
    }
    std::lock_guard<std::mutex> autoLock(dataLock_);
    autoLaunchItemMap_[identifier].inObserver = false;
    cv_.notify_all();
    LOGI("[AutoLaunch] ObserverFunc finished");
}

int AutoLaunch::DisableKvStoreAutoLaunch(const std::string &identifier)
{
    LOGI("[AutoLaunch] DisableKvStoreAutoLaunch");
    AutoLaunchItem autoLaunchItem;
    {
        std::unique_lock<std::mutex> autoLock(dataLock_);
        if (autoLaunchItemMap_.count(identifier) == 0) {
            LOGE("[AutoLaunch] DisableKvStoreAutoLaunch identifier is not exist!");
            return -E_NOT_FOUND;
        }
        if (autoLaunchItemMap_[identifier].isDisable == true) {
            LOGI("[AutoLaunch] DisableKvStoreAutoLaunch already disabling in another thread, do nothing here");
            return -E_BUSY;
        }
        if (autoLaunchItemMap_[identifier].state == AutoLaunchItemState::IN_ENABLE) {
            LOGE("[AutoLaunch] DisableKvStoreAutoLaunch enable not return, do not disable!");
            return -E_BUSY;
        }
        autoLaunchItemMap_[identifier].isDisable = true;
        if (autoLaunchItemMap_[identifier].state != AutoLaunchItemState::IDLE) {
            LOGI("[AutoLaunch] DisableKvStoreAutoLaunch wait idle");
            cv_.wait(autoLock, [identifier, this] {
                return (autoLaunchItemMap_[identifier].state == AutoLaunchItemState::IDLE) &&
                    (!autoLaunchItemMap_[identifier].inObserver);
            });
            LOGI("[AutoLaunch] DisableKvStoreAutoLaunch wait idle ok");
        }
        autoLaunchItem = autoLaunchItemMap_[identifier];
    }

    int errCode = CloseConnectionStrict(autoLaunchItem);
    if (errCode == E_OK) {
        std::unique_lock<std::mutex> autoLock(dataLock_);
        autoLaunchItemMap_.erase(identifier);
        cv_.notify_all();
        LOGI("[AutoLaunch] DisableKvStoreAutoLaunch CloseConnection ok");
    } else {
        LOGE("[AutoLaunch] DisableKvStoreAutoLaunch CloseConnection failed errCode:%d", errCode);
        std::unique_lock<std::mutex> autoLock(dataLock_);
        autoLaunchItemMap_[identifier].isDisable = false;
        autoLaunchItemMap_[identifier].observerHandle = autoLaunchItem.observerHandle;
        cv_.notify_all();
        return errCode;
    }
    if (autoLaunchItem.isWriteOpenNotifiered && autoLaunchItem.notifier) {
        RuntimeContext::GetInstance()->ScheduleTask([autoLaunchItem, this] { CloseNotifier(autoLaunchItem); });
    }
    LOGI("[AutoLaunch] DisableKvStoreAutoLaunch ok");
    return E_OK;
}

void AutoLaunch::GetAutoLaunchSyncDevices(const std::string &identifier, std::vector<std::string> &devices) const
{
    devices.clear();
    devices.shrink_to_fit();
    std::lock_guard<std::mutex> autoLock(dataLock_);
    if (autoLaunchItemMap_.count(identifier) == 0) {
        LOGD("[AutoLaunch] GetSyncDevices identifier is not exist!");
        return;
    }
    for (const auto &device : onlineDevices_) {
        devices.push_back(device);
    }
}

void AutoLaunch::CloseNotifier(const AutoLaunchItem &autoLaunchItem)
{
    if (autoLaunchItem.notifier) {
        std::string userId = autoLaunchItem.properties.GetStringProp(KvDBProperties::USER_ID, "");
        std::string appId = autoLaunchItem.properties.GetStringProp(KvDBProperties::APP_ID, "");
        std::string storeId = autoLaunchItem.properties.GetStringProp(KvDBProperties::STORE_ID, "");
        LOGI("[AutoLaunch] CloseNotifier do autoLaunchItem.notifier");
        autoLaunchItem.notifier(userId, appId, storeId, AutoLaunchStatus::WRITE_CLOSED);
        LOGI("[AutoLaunch] CloseNotifier do autoLaunchItem.notifier finished");
    } else {
        LOGI("[AutoLaunch] CloseNotifier autoLaunchItem.notifier is nullptr");
    }
}

void AutoLaunch::ConnectionLifeCycleCallbackTask(const std::string &identifier)
{
    LOGI("[AutoLaunch] ConnectionLifeCycleCallbackTask");
    AutoLaunchItem autoLaunchItem;
    {
        std::lock_guard<std::mutex> autoLock(dataLock_);
        if (autoLaunchItemMap_.count(identifier) == 0) {
            LOGE("[AutoLaunch] ConnectionLifeCycleCallback identifier is not exist!");
            return;
        }
        if (autoLaunchItemMap_[identifier].isDisable) {
            LOGI("[AutoLaunch] ConnectionLifeCycleCallback isDisable, do nothing");
            return;
        }
        if (autoLaunchItemMap_[identifier].state != AutoLaunchItemState::IDLE) {
            LOGI("[AutoLaunch] ConnectionLifeCycleCallback state:%d is not idle, do nothing",
                autoLaunchItemMap_[identifier].state);
            return;
        }
        autoLaunchItemMap_[identifier].state = AutoLaunchItemState::IN_LIFE_CYCLE_CALL_BACK;
        autoLaunchItem = autoLaunchItemMap_[identifier];
    }
    LOGI("[AutoLaunch] ConnectionLifeCycleCallbackTask do CloseConnection");
    TryCloseConnection(autoLaunchItem); // do onthing if failed
    LOGI("[AutoLaunch] ConnectionLifeCycleCallback do CloseConnection finished");
    {
        std::lock_guard<std::mutex> lock(dataLock_);
        autoLaunchItemMap_[identifier].state = AutoLaunchItemState::IDLE;
        autoLaunchItemMap_[identifier].conn = nullptr;
        autoLaunchItemMap_[identifier].isWriteOpenNotifiered = false;
        cv_.notify_all();
        LOGI("[AutoLaunch] ConnectionLifeCycleCallback notify_all");
    }
    if (autoLaunchItem.isWriteOpenNotifiered) {
        CloseNotifier(autoLaunchItem);
    }
}

void AutoLaunch::ConnectionLifeCycleCallback(const std::string &identifier)
{
    LOGI("[AutoLaunch] ConnectionLifeCycleCallback");
    int errCode = RuntimeContext::GetInstance()->ScheduleTask(std::bind(&AutoLaunch::ConnectionLifeCycleCallbackTask,
        this, identifier));
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] ConnectionLifeCycleCallback ScheduleTask failed");
    }
}

IKvDBConnection *AutoLaunch::GetOneConnection(const KvDBProperties &properties, int &errCode)
{
    LOGI("[AutoLaunch] GetOneConnection");
    IKvDBConnection *conn = KvDBManager::GetDatabaseConnection(properties, errCode, false);
    if (errCode == -E_ALREADY_OPENED) {
        LOGI("[AutoLaunch] GetOneConnection user already getkvstore by self");
    } else if (conn == nullptr) {
        LOGE("[AutoLaunch] GetOneConnection GetDatabaseConnection failed errCode:%d", errCode);
    }
    return conn;
}

void AutoLaunch::OnlineCallBack(const std::string &device, bool isConnect)
{
    LOGI("[AutoLaunch] OnlineCallBack device:%s{private}, isConnect:%d", device.c_str(), isConnect);
    if (!isConnect) {
        std::lock_guard<std::mutex> autoLock(dataLock_);
        onlineDevices_.erase(device);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(dataLock_);
        onlineDevices_.insert(device);
    }

    int errCode = RuntimeContext::GetInstance()->ScheduleTask(std::bind(&AutoLaunch::OnlineCallBackTask, this));
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] OnlineCallBack ScheduleTask failed");
    }
}

void AutoLaunch::OnlineCallBackTask()
{
    LOGI("[AutoLaunch] OnlineCallBackTask");
    std::map<std::string, AutoLaunchItem> doOpenMap;
    GetDoOpenMap(doOpenMap);
    GetConnInDoOpenMap(doOpenMap);
    UpdateGlobalMap(doOpenMap);
}

void AutoLaunch::GetDoOpenMap(std::map<std::string, AutoLaunchItem> &doOpenMap)
{
    std::lock_guard<std::mutex> autoLock(dataLock_);
    LOGI("[AutoLaunch] GetDoOpenMap");
    for (auto &iter : autoLaunchItemMap_) {
        if (iter.second.isDisable) {
            LOGI("[AutoLaunch] GetDoOpenMap this item isDisable do nothing");
            continue;
        } else if (iter.second.state != AutoLaunchItemState::IDLE) {
            LOGI("[AutoLaunch] GetDoOpenMap this item state:%d is not idle do nothing", iter.second.state);
            continue;
        } else if (iter.second.conn != nullptr) {
            LOGI("[AutoLaunch] GetDoOpenMap this item is opened");
            continue;
        } else {
            doOpenMap[iter.first] = iter.second;
            iter.second.state = AutoLaunchItemState::IN_COMMUNICATOR_CALL_BACK;
            LOGI("[AutoLaunch] GetDoOpenMap set state IN_COMMUNICATOR_CALL_BACK");
        }
    }
}

void AutoLaunch::GetConnInDoOpenMap(std::map<std::string, AutoLaunchItem> &doOpenMap)
{
    LOGI("[AutoLaunch] GetConnInDoOpenMap doOpenMap.size():%llu", doOpenMap.size());
    if (doOpenMap.empty()) {
        return;
    }
    Semaphore sema(1 - doOpenMap.size());
    for (auto &iter : doOpenMap) {
        int errCode = RuntimeContext::GetInstance()->ScheduleTask([&sema, &iter, this] {
            int errCode;
            iter.second.conn = GetOneConnection(iter.second.properties, errCode);
            LOGI("[AutoLaunch] GetConnInDoOpenMap GetOneConnection errCode:%d\n", errCode);
            if (iter.second.conn == nullptr) {
                sema.SendSemaphore();
                LOGI("[AutoLaunch] GetConnInDoOpenMap in open thread finish SendSemaphore");
                return;
            }
            errCode = RegisterObserverAndLifeCycleCallback(iter.second, iter.first, false);
            if (errCode != E_OK) {
                LOGE("[AutoLaunch] GetConnInDoOpenMap  failed, we do CloseConnection");
                TryCloseConnection(iter.second); // if here failed, do nothing
                iter.second.conn = nullptr;
            }
            sema.SendSemaphore();
            LOGI("[AutoLaunch] GetConnInDoOpenMap in open thread finish SendSemaphore");
        });
        if (errCode != E_OK) {
            LOGE("[AutoLaunch] GetConnInDoOpenMap ScheduleTask failed, SendSemaphore");
            sema.SendSemaphore();
        }
    }
    LOGI("[AutoLaunch] GetConnInDoOpenMap WaitSemaphore");
    sema.WaitSemaphore();
    LOGI("[AutoLaunch] GetConnInDoOpenMap WaitSemaphore ok");
}

void AutoLaunch::UpdateGlobalMap(std::map<std::string, AutoLaunchItem> &doOpenMap)
{
    std::lock_guard<std::mutex> autoLock(dataLock_);
    LOGI("[AutoLaunch] UpdateGlobalMap");
    for (auto &iter : doOpenMap) {
        if (iter.second.conn != nullptr) {
            autoLaunchItemMap_[iter.first].conn = iter.second.conn;
            autoLaunchItemMap_[iter.first].observerHandle = iter.second.observerHandle;
            autoLaunchItemMap_[iter.first].isWriteOpenNotifiered = false;
            LOGI("[AutoLaunch] UpdateGlobalMap opened conn update map");
        }
        autoLaunchItemMap_[iter.first].state = AutoLaunchItemState::IDLE;
        LOGI("[AutoLaunch] UpdateGlobalMap opened conn set state IDLE");
    }
    cv_.notify_all();
    LOGI("[AutoLaunch] UpdateGlobalMap finish notify_all");
}

void AutoLaunch::ReceiveUnknownIdentifierCallBackTask(const std::string &identifier)
{
    LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBackTask");
    AutoLaunchItem autoLaunchItem;
    {
        std::lock_guard<std::mutex> autoLock(dataLock_);
        autoLaunchItem = autoLaunchItemMap_[identifier];
    }
    int errCode;
    autoLaunchItem.conn = GetOneConnection(autoLaunchItem.properties, errCode);
    LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBack GetOneConnection errCode:%d\n", errCode);
    if (autoLaunchItem.conn == nullptr) {
        std::lock_guard<std::mutex> autoLock(dataLock_);
        autoLaunchItemMap_[identifier].state = AutoLaunchItemState::IDLE;
        cv_.notify_all();
        LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBackTask set state IDLE");
        return;
    }
    errCode = RegisterObserverAndLifeCycleCallback(autoLaunchItem, identifier, false);
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] ReceiveUnknownIdentifierCallBackTask RegisterObserverAndLifeCycleCallback failed");
        LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBackTask do CloseConnection");
        TryCloseConnection(autoLaunchItem); // if here failed, do nothing
        std::lock_guard<std::mutex> autoLock(dataLock_);
        autoLaunchItemMap_[identifier].state = AutoLaunchItemState::IDLE;
        cv_.notify_all();
        LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBackTask set state IDLE");
        return;
    }
    std::lock_guard<std::mutex> autoLock(dataLock_);
    autoLaunchItemMap_[identifier].conn = autoLaunchItem.conn;
    autoLaunchItemMap_[identifier].observerHandle = autoLaunchItem.observerHandle;
    autoLaunchItemMap_[identifier].isWriteOpenNotifiered = false;
    autoLaunchItemMap_[identifier].state = AutoLaunchItemState::IDLE;
    cv_.notify_all();
    LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBackTask conn opened set state IDLE");
}

int AutoLaunch::ReceiveUnknownIdentifierCallBack(const LabelType &label)
{
    const std::string identifier(label.begin(), label.end());
    LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBack");
    int errCode;
    {
        std::lock_guard<std::mutex> autoLock(dataLock_);
        if (autoLaunchItemMap_.count(identifier) == 0) {
            LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBack not find identifier");
            goto EXT;
        } else if (autoLaunchItemMap_[identifier].isDisable) {
            LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBack isDisable ,do nothing");
            return -E_NOT_FOUND; // not E_OK is ok for communicator
        } else if (autoLaunchItemMap_[identifier].conn != nullptr) {
            LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBack conn is not nullptr");
            return E_OK;
        } else if (autoLaunchItemMap_[identifier].state != AutoLaunchItemState::IDLE) {
            LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBack state:%d is not idle, do nothing",
                autoLaunchItemMap_[identifier].state);
            return E_OK;
        }
        autoLaunchItemMap_[identifier].state = AutoLaunchItemState::IN_COMMUNICATOR_CALL_BACK;
        LOGI("[AutoLaunch] ReceiveUnknownIdentifierCallBack set state IN_COMMUNICATOR_CALL_BACK");
    }

    errCode = RuntimeContext::GetInstance()->ScheduleTask(std::bind(
        &AutoLaunch::ReceiveUnknownIdentifierCallBackTask, this, identifier));
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] ReceiveUnknownIdentifierCallBack ScheduleTask failed");
        std::lock_guard<std::mutex> autoLock(dataLock_);
        autoLaunchItemMap_[identifier].state = AutoLaunchItemState::IDLE;
    }
    return errCode;

EXT:
    return AutoLaunchExt(identifier);
}

void AutoLaunch::SetAutoLaunchRequestCallback(const AutoLaunchRequestCallback &callback)
{
    LOGI("[AutoLaunch] SetAutoLaunchRequestCallback");
    std::lock_guard<std::mutex> lock(extLock_);
    autoLaunchRequestCallback_ = callback;
}

int AutoLaunch::AutoLaunchExt(const std::string &identifier)
{
    AutoLaunchParam param;
    {
        std::lock_guard<std::mutex> lock(extLock_);
        if (!autoLaunchRequestCallback_) {
            LOGI("[AutoLaunch] autoLaunchRequestCallback_ is nullptr");
            return -E_NOT_FOUND; // not E_OK is ok for communicator
        }
        bool needOpen = autoLaunchRequestCallback_(identifier, param);
        if (!needOpen) {
            LOGI("[AutoLaunch] autoLaunchRequestCallback_ is not need open");
            return -E_NOT_FOUND; // not E_OK is ok for communicator
        }
    }
    KvDBProperties properties;
    int errCode = AutoLaunch::GetAutoLaunchProperties(param, properties);
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] AutoLaunchExt param check fail errCode:%d", errCode);
        if (!param.notifier) {
            return errCode;
        }
        int retCode = RuntimeContext::GetInstance()->ScheduleTask([param] {
            param.notifier(param.userId, param.appId, param.storeId, INVALID_PARAM);
        });
        if (retCode != E_OK) {
            LOGE("[AutoLaunch] AutoLaunchExt notifier ScheduleTask retCode:%d", retCode);
        }
        return errCode;
    }
    AutoLaunchItem autoLaunchItem{properties, param.notifier, param.option.observer, param.option.conflictType,
        param.option.notifier};
    errCode = RuntimeContext::GetInstance()->ScheduleTask(std::bind(&AutoLaunch::AutoLaunchExtTask, this,
        identifier, autoLaunchItem));
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] AutoLaunchExt ScheduleTask errCode:%d", errCode);
    }
    return errCode;
}

void AutoLaunch::AutoLaunchExtTask(const std::string identifier, AutoLaunchItem autoLaunchItem)
{
    {
        std::lock_guard<std::mutex> autoLock(extLock_);
        if (extItemMap_.count(identifier) != 0) {
            LOGE("[AutoLaunch] extItemMap_ has this identifier");
            return;
        }
        extItemMap_[identifier] = autoLaunchItem;
    }
    int errCode;
    autoLaunchItem.conn = GetOneConnection(autoLaunchItem.properties, errCode);
    LOGI("[AutoLaunch] AutoLaunchExtTask GetOneConnection errCode:%d", errCode);
    if (autoLaunchItem.conn == nullptr) {
        std::lock_guard<std::mutex> autoLock(extLock_);
        extItemMap_.erase(identifier);
        return;
    }

    errCode = RegisterObserverAndLifeCycleCallback(autoLaunchItem, identifier, true);
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] AutoLaunchExtTask RegisterObserverAndLifeCycleCallback failed");
        TryCloseConnection(autoLaunchItem); // if here failed, do nothing
        std::lock_guard<std::mutex> autoLock(extLock_);
        extItemMap_.erase(identifier);
        return;
    }
    std::lock_guard<std::mutex> autoLock(extLock_);
    extItemMap_[identifier].conn = autoLaunchItem.conn;
    extItemMap_[identifier].observerHandle = autoLaunchItem.observerHandle;
    extItemMap_[identifier].isWriteOpenNotifiered = false;
    LOGI("[AutoLaunch] AutoLaunchExtTask ok");
}

void AutoLaunch::ExtObserverFunc(const KvDBCommitNotifyData &notifyData, const std::string &identifier)
{
    LOGD("[AutoLaunch] ExtObserverFunc");
    AutoLaunchItem autoLaunchItem;
    AutoLaunchNotifier notifier;
    {
        std::lock_guard<std::mutex> autoLock(extLock_);
        if (extItemMap_.count(identifier) == 0) {
            LOGE("[AutoLaunch] ExtObserverFunc this identifier not in map");
            return;
        }
        autoLaunchItem = extItemMap_[identifier];
    }
    if (autoLaunchItem.observer != nullptr) {
        LOGD("[AutoLaunch] do user observer");
        KvStoreChangedDataImpl data(&notifyData);
        autoLaunchItem.observer->OnChange(data);
    }

    {
        std::lock_guard<std::mutex> autoLock(extLock_);
        if (extItemMap_.count(identifier) != 0 && !extItemMap_[identifier].isWriteOpenNotifiered &&
            autoLaunchItem.notifier != nullptr) {
            extItemMap_[identifier].isWriteOpenNotifiered = true;
            notifier = autoLaunchItem.notifier;
        } else {
            return;
        }
    }

    std::string userId = autoLaunchItem.properties.GetStringProp(KvDBProperties::USER_ID, "");
    std::string appId = autoLaunchItem.properties.GetStringProp(KvDBProperties::APP_ID, "");
    std::string storeId = autoLaunchItem.properties.GetStringProp(KvDBProperties::STORE_ID, "");
    int retCode = RuntimeContext::GetInstance()->ScheduleTask([notifier, userId, appId, storeId] {
        LOGI("[AutoLaunch] ExtObserverFunc do user notifier WRITE_OPENED");
        notifier(userId, appId, storeId, AutoLaunchStatus::WRITE_OPENED);
    });
    if (retCode != E_OK) {
        LOGE("[AutoLaunch] ExtObserverFunc notifier ScheduleTask retCode:%d", retCode);
    }
}

void AutoLaunch::ExtConnectionLifeCycleCallback(const std::string &identifier)
{
    LOGI("[AutoLaunch] ExtConnectionLifeCycleCallback");
    int errCode = RuntimeContext::GetInstance()->ScheduleTask(std::bind(
        &AutoLaunch::ExtConnectionLifeCycleCallbackTask, this, identifier));
    if (errCode != E_OK) {
        LOGE("[AutoLaunch] ExtConnectionLifeCycleCallback ScheduleTask failed");
    }
}

void AutoLaunch::ExtConnectionLifeCycleCallbackTask(const std::string &identifier)
{
    LOGI("[AutoLaunch] ExtConnectionLifeCycleCallbackTask");
    AutoLaunchItem autoLaunchItem;
    {
        std::lock_guard<std::mutex> autoLock(extLock_);
        if (extItemMap_.count(identifier) == 0) {
            LOGE("[AutoLaunch] ExtConnectionLifeCycleCallbackTask identifier is not exist!");
            return;
        }
        autoLaunchItem = extItemMap_[identifier];
    }
    LOGI("[AutoLaunch] ExtConnectionLifeCycleCallbackTask do CloseConnection");
    TryCloseConnection(autoLaunchItem); // do nothing if failed
    {
        std::lock_guard<std::mutex> lock(extLock_);
        autoLaunchItem = extItemMap_[identifier];
        extItemMap_.erase(identifier);
    }
    if (autoLaunchItem.isWriteOpenNotifiered) {
        CloseNotifier(autoLaunchItem);
    }
}

int AutoLaunch::SetConflictNotifier(IKvDBConnection *conn, int conflictType, const KvStoreNbConflictNotifier &notifier)
{
    if (conflictType == 0) {
        return E_OK;
    }
    int errCode;
    if (!notifier) {
        errCode = conn->SetConflictNotifier(conflictType, nullptr);
        goto END;
    }

    errCode = conn->SetConflictNotifier(conflictType,
        [conflictType, notifier](const KvDBCommitNotifyData &data) {
            int resultCode;
            const std::list<KvDBConflictEntry> entries = data.GetCommitConflicts(resultCode);
            if (resultCode != E_OK) {
                LOGE("Get commit conflicted entries failed:%d!", resultCode);
                return;
            }

            for (const auto &entry : entries) {
                // Prohibit signed numbers to perform bit operations
                uint32_t entryType = static_cast<uint32_t>(entry.type);
                uint32_t type = static_cast<uint32_t>(conflictType);
                if ((entryType & type) != 0) {
                    KvStoreNbConflictDataImpl dataImpl;
                    dataImpl.SetConflictData(entry);
                    notifier(dataImpl);
                }
            }
        });

END:
    if (errCode != E_OK) {
        LOGE("[KvStoreNbDelegate] Register conflict failed:%d!", errCode);
    }
    return errCode;
}

int AutoLaunch::GetAutoLaunchProperties(const AutoLaunchParam &param, KvDBProperties &properties)
{
    SchemaObject schemaObject;
    std::string canonicalDir;
    int errCode = ParamCheckUtils::CheckAndTransferAutoLaunchParam(param, schemaObject, canonicalDir);
    if (errCode != E_OK) {
        return errCode;
    }

    if (param.option.isEncryptedDb) {
        properties.SetPassword(param.option.cipher, param.option.passwd);
    }
    properties.SetStringProp(KvDBProperties::DATA_DIR, canonicalDir);
    properties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, param.option.createIfNecessary);
    properties.SetBoolProp(KvDBProperties::CREATE_DIR_BY_STORE_ID_ONLY, param.option.createDirByStoreIdOnly);
    properties.SetBoolProp(KvDBProperties::MEMORY_MODE, false);
    properties.SetBoolProp(KvDBProperties::ENCRYPTED_MODE, param.option.isEncryptedDb);
    properties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
    properties.SetSchema(schemaObject);
    if (RuntimeContext::GetInstance()->IsProcessSystemApiAdapterValid()) {
        properties.SetIntProp(KvDBProperties::SECURITY_LABEL, param.option.secOption.securityLabel);
        properties.SetIntProp(KvDBProperties::SECURITY_FLAG, param.option.secOption.securityFlag);
    }
    DBCommon::SetDatabaseIds(properties, param.appId, param.userId, param.storeId);
    return E_OK;
}
} // namespace DistributedDB
