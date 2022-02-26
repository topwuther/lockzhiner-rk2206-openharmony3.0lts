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

#ifndef AUTO_LAUNCH_H
#define AUTO_LAUNCH_H

#include <set>
#include <map>
#include <mutex>
#include "types_export.h"
#include "kv_store_observer.h"
#include "kvdb_properties.h"
#include "ikvdb_connection.h"
#include "icommunicator_aggregator.h"
#include "auto_launch_export.h"

namespace DistributedDB {
enum class AutoLaunchItemState {
    UN_INITIAL = 0,
    IN_ENABLE,
    IN_LIFE_CYCLE_CALL_BACK, // in LifeCycleCallback
    IN_COMMUNICATOR_CALL_BACK, // in OnConnectCallback or CommunicatorLackCallback
    IDLE,
};

struct AutoLaunchItem {
    KvDBProperties properties;
    AutoLaunchNotifier notifier;
    KvStoreObserver *observer = nullptr;
    int conflictType = 0;
    KvStoreNbConflictNotifier conflictNotifier;
    IKvDBConnection *conn = nullptr;
    KvDBObserverHandle *observerHandle = nullptr;
    bool isWriteOpenNotifiered = false;
    AutoLaunchItemState state = AutoLaunchItemState::UN_INITIAL;
    bool isDisable = false;
    bool inObserver = false;
};

class AutoLaunch {
public:
    AutoLaunch() = default;

    ~AutoLaunch();

    DISABLE_COPY_ASSIGN_MOVE(AutoLaunch);

    void SetCommunicatorAggregator(ICommunicatorAggregator *aggregator);

    int EnableKvStoreAutoLaunch(const KvDBProperties &properties, AutoLaunchNotifier notifier,
        KvStoreObserver *observer, int conflictType, KvStoreNbConflictNotifier conflictNotifier);

    int DisableKvStoreAutoLaunch(const std::string &identifier);

    void GetAutoLaunchSyncDevices(const std::string &identifier, std::vector<std::string> &devices) const;

    void SetAutoLaunchRequestCallback(const AutoLaunchRequestCallback &callback);

    static int GetAutoLaunchProperties(const AutoLaunchParam &param, KvDBProperties &properties);

private:

    int EnableKvStoreAutoLaunchParmCheck(AutoLaunchItem &autoLaunchItem, const std::string &identifier);

    int GetConnectionInEnable(AutoLaunchItem &autoLaunchItem, const std::string &identifier);

    IKvDBConnection *GetOneConnection(const KvDBProperties &properties, int &errCode);

    // we will return errCode, if errCode != E_OK
    int CloseConnectionStrict(AutoLaunchItem &autoLaunchItem);

    // before ReleaseDatabaseConnection, if errCode != E_OK, we not return, we try close more
    void TryCloseConnection(AutoLaunchItem &autoLaunchItem);

    int RegisterObserverAndLifeCycleCallback(AutoLaunchItem &autoLaunchItem, const std::string &identifier,
        bool isExt);

    int RegisterObserver(AutoLaunchItem &autoLaunchItem, const std::string &identifier, bool isExt);

    void ObserverFunc(const KvDBCommitNotifyData &notifyData, const std::string &identifier);

    void ConnectionLifeCycleCallbackTask(const std::string &identifier);

    void OnlineCallBackTask();

    void GetDoOpenMap(std::map<std::string, AutoLaunchItem> &doOpenMap);

    void GetConnInDoOpenMap(std::map<std::string, AutoLaunchItem> &doOpenMap);

    void UpdateGlobalMap(std::map<std::string, AutoLaunchItem> &doOpenMap);

    void ReceiveUnknownIdentifierCallBackTask(const std::string &identifier);

    void CloseNotifier(const AutoLaunchItem &autoLaunchItem);

    void ConnectionLifeCycleCallback(const std::string &identifier);

    void OnlineCallBack(const std::string &device, bool isConnect);

    int ReceiveUnknownIdentifierCallBack(const LabelType &label);

    int AutoLaunchExt(const std::string &identifier);

    void AutoLaunchExtTask(const std::string identifier, AutoLaunchItem autoLaunchItem);

    void ExtObserverFunc(const KvDBCommitNotifyData &notifyData, const std::string &identifier);

    void ExtConnectionLifeCycleCallback(const std::string &identifier);

    void ExtConnectionLifeCycleCallbackTask(const std::string &identifier);

    int SetConflictNotifier(IKvDBConnection *conn, int conflictType, const KvStoreNbConflictNotifier &notifier);

    mutable std::mutex dataLock_;
    mutable std::mutex communicatorLock_;
    std::set<std::string> onlineDevices_;
    std::map<std::string, AutoLaunchItem> autoLaunchItemMap_;
    ICommunicatorAggregator *communicatorAggregator_ = nullptr;
    std::condition_variable cv_;

    std::mutex extLock_;
    AutoLaunchRequestCallback autoLaunchRequestCallback_;
    std::map<std::string, AutoLaunchItem> extItemMap_;
};
} // namespace DistributedDB
#endif // AUTO_LAUNCH_H
