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

#ifndef RUNTIME_CONTEXT_H
#define RUNTIME_CONTEXT_H

#include <cstdint>
#include <string>
#include <functional>

#include "macro_utils.h"
#include "notification_chain.h"
#include "icommunicator_aggregator.h"
#include "iprocess_system_api_adapter.h"
#include "types_export.h"
#include "kv_store_observer.h"
#include "kvdb_properties.h"
#include "auto_launch_export.h"

namespace DistributedDB {
using TimerId = uint64_t;
using TimerAction = std::function<int(TimerId timerId)>;
using TimerFinalizer = std::function<void(void)>;
using TaskAction = std::function<void(void)>;
using TimeChangedAction = std::function<void(void *)>;
using LockStatusNotifier = std::function<void(void *isLocked)>;

class RuntimeContext {
public:
    DISABLE_COPY_ASSIGN_MOVE(RuntimeContext);

    // Global setting interfaces.
    virtual void SetProcessLabel(const std::string &label) = 0;
    virtual std::string GetProcessLabel() const = 0;
    // If the pre adapter is not nullptr, set new adapter will release the pre adapter,
    // must be called after SetCommunicatorAggregator
    virtual int SetCommunicatorAdapter(IAdapter *adapter) = 0;
    virtual int GetCommunicatorAggregator(ICommunicatorAggregator *&outAggregator) = 0;
    virtual void SetCommunicatorAggregator(ICommunicatorAggregator *inAggregator) = 0;

    // Timer interfaces.
    virtual int SetTimer(int milliSeconds, const TimerAction &action,
        const TimerFinalizer &finalizer, TimerId &timerId) = 0;
    virtual int ModifyTimer(TimerId timerId, int milliSeconds) = 0;
    virtual void RemoveTimer(TimerId timerId, bool wait = false) = 0;

    // Task interfaces.
    virtual int ScheduleTask(const TaskAction &task) = 0;
    virtual int ScheduleQueuedTask(const std::string &queueTag,
        const TaskAction &task) = 0;

    // Shrink as much memory as possible.
    virtual void ShrinkMemory(const std::string &description) = 0;

    // Register a time changed lister, it will be callback when local time changed.
    virtual NotificationChain::Listener *RegisterTimeChangedLister(const TimeChangedAction &action, int &errCode) = 0;

    // Get the global context object(singleton), never return nullptr.
    static RuntimeContext *GetInstance();

    virtual int SetPermissionCheckCallback(const PermissionCheckCallback &callback) = 0;

    virtual int SetPermissionCheckCallback(const PermissionCheckCallbackV2 &callback) = 0;

    virtual int RunPermissionCheck(const std::string &userId, const std::string &appId, const std::string &storeId,
        const std::string &deviceId, uint8_t flag) const = 0;

    virtual int EnableKvStoreAutoLaunch(const KvDBProperties &properties, AutoLaunchNotifier notifier,
        KvStoreObserver *observer, int conflictType, KvStoreNbConflictNotifier conflictNotifier) = 0;

    virtual int DisableKvStoreAutoLaunch(const std::string &identifier) = 0;

    virtual void GetAutoLaunchSyncDevices(const std::string &identifier, std::vector<std::string> &devices) const = 0;

    virtual void SetAutoLaunchRequestCallback(const AutoLaunchRequestCallback &callback) = 0;

    virtual NotificationChain::Listener *RegisterLockStatusLister(const LockStatusNotifier &action, int &errorCode) = 0;

    virtual bool IsAccessControlled() const = 0;

    virtual int SetSecurityOption(const std::string &filePath, const SecurityOption &option) const = 0;

    virtual int GetSecurityOption(const std::string &filePath, SecurityOption &option) const = 0;

    virtual bool CheckDeviceSecurityAbility(const std::string &devId, const SecurityOption &option) const = 0;

    virtual int SetProcessSystemApiAdapter(const std::shared_ptr<IProcessSystemApiAdapter> &adapter) = 0;

    virtual bool IsProcessSystemApiAdapterValid() const = 0;

    virtual bool IsCommunicatorAggregatorValid() const = 0;

    // Notify TIME_CHANGE_EVENT.
    virtual void NotifyTimeStampChanged(TimeOffset offset) const = 0;
protected:
    RuntimeContext() = default;
    virtual ~RuntimeContext() {}
};
} // namespace DistributedDB

#endif // RUNTIME_CONTEXT_H
