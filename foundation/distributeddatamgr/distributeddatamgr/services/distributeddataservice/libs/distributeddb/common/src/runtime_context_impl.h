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

#ifndef RUNTIME_CONTEXT_IMPL_H
#define RUNTIME_CONTEXT_IMPL_H

#include <mutex>
#include <map>
#include <shared_mutex>

#include "runtime_context.h"
#include "task_pool.h"
#include "evloop/include/ievent.h"
#include "evloop/include/ievent_loop.h"
#include "lock_status_observer.h"
#include "time_tick_monitor.h"
#include "icommunicator_aggregator.h"
#include "auto_launch.h"

namespace DistributedDB {
class RuntimeContextImpl final : public RuntimeContext {
public:
    RuntimeContextImpl();
    ~RuntimeContextImpl() override;

    // Get/Set the label of this process.
    void SetProcessLabel(const std::string &label) override;
    std::string GetProcessLabel() const override;
    int SetCommunicatorAdapter(IAdapter *adapter) override;
    int GetCommunicatorAggregator(ICommunicatorAggregator *&outAggregator) override;
    void SetCommunicatorAggregator(ICommunicatorAggregator *inAggregator) override;

    // Add and start a timer.
    int SetTimer(int milliSeconds, const TimerAction &action,
        const TimerFinalizer &finalizer, TimerId &timerId) override;

    // Modify the interval of the timer.
    int ModifyTimer(TimerId timerId, int milliSeconds) override;

    // Remove the timer.
    void RemoveTimer(TimerId timerId, bool wait) override;

    // Task interfaces.
    int ScheduleTask(const TaskAction &task) override;
    int ScheduleQueuedTask(const std::string &queueTag, const TaskAction &task) override;

    // Shrink as much memory as possible.
    void ShrinkMemory(const std::string &description) override;

    // Register a time changed lister, it will be callback when local time changed.
    NotificationChain::Listener *RegisterTimeChangedLister(const TimeChangedAction &action, int &errCode) override;

    int SetPermissionCheckCallback(const PermissionCheckCallback &callback) override;

    int SetPermissionCheckCallback(const PermissionCheckCallbackV2 &callback) override;

    int RunPermissionCheck(const std::string &userId, const std::string &appId, const std::string &storeId,
        const std::string &deviceId, uint8_t flag) const override;

    int EnableKvStoreAutoLaunch(const KvDBProperties &properties, AutoLaunchNotifier notifier,
        KvStoreObserver *observer, int conflictType, KvStoreNbConflictNotifier conflictNotifier) override;

    int DisableKvStoreAutoLaunch(const std::string &identifier) override;

    void GetAutoLaunchSyncDevices(const std::string &identifier, std::vector<std::string> &devices) const override;

    void SetAutoLaunchRequestCallback(const AutoLaunchRequestCallback &callback) override;

    NotificationChain::Listener *RegisterLockStatusLister(const LockStatusNotifier &action, int &errorCode) override;

    bool IsAccessControlled() const override;

    int SetSecurityOption(const std::string &filePath, const SecurityOption &option) const override;

    int GetSecurityOption(const std::string &filePath, SecurityOption &option) const override;

    bool CheckDeviceSecurityAbility(const std::string &devId, const SecurityOption &option) const override;

    int SetProcessSystemApiAdapter(const std::shared_ptr<IProcessSystemApiAdapter> &adapter) override;

    bool IsProcessSystemApiAdapterValid() const override;

    bool IsCommunicatorAggregatorValid() const override;
    // Notify TIME_CHANGE_EVENT.
    void NotifyTimeStampChanged(TimeOffset offset) const override;

private:
    static constexpr int MAX_TP_THREADS = 10;  // max threads of the task pool.
    static constexpr int MIN_TP_THREADS = 1;   // min threads of the task pool.
    static constexpr int TASK_POOL_REPORTS_INTERVAL = 10000;   // task pool reports its state every 10 seconds.

    int PrepareLoop(IEventLoop *&loop);
    int PrepareTaskPool();
    int AllocTimerId(IEvent *evTimer, TimerId &timerId);

    // Context fields
    mutable std::mutex labelMutex_;
    std::string processLabel_;

    // Communicator
    mutable std::mutex communicatorLock_;
    IAdapter *adapter_;
    ICommunicatorAggregator *communicatorAggregator_;

    // Loop and timer
    mutable std::mutex loopLock_;
    IEventLoop *mainLoop_;
    std::mutex timersLock_;
    TimerId currentTimerId_;
    std::map<TimerId, IEvent *> timers_;

    // Task pool
    std::mutex taskLock_;
    TaskPool *taskPool_;
    TimerId taskPoolReportsTimerId_;

    // TimeTick
    mutable std::mutex timeTickMonitorLock_;
    std::unique_ptr<TimeTickMonitor> timeTickMonitor_;

    mutable std::shared_mutex permissionCheckCallbackMutex_{};
    PermissionCheckCallback permissionCheckCallback_;
    PermissionCheckCallbackV2 permissionCheckCallbackV2_;

    AutoLaunch autoLaunch_;

    // System api
    mutable std::mutex systemApiAdapterLock_;
    std::shared_ptr<IProcessSystemApiAdapter> systemApiAdapter_;
    mutable std::mutex lockStatusLock_; // Mutex for lockStatusObserver_.
    LockStatusObserver *lockStatusObserver_;
};
} // namespace DistributedDB

#endif // RUNTIME_CONTEXT_IMPL_H
