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

#ifndef SYNC_ABLE_KVDB_CONNECTION_H
#define SYNC_ABLE_KVDB_CONNECTION_H

#include "ref_object.h"
#include "generic_kvdb_connection.h"

namespace DistributedDB {
class SyncAbleKvDB;
struct PragmaSync;
struct PragmaRemotePushNotify;

class SyncAbleKvDBConnection : public GenericKvDBConnection, public virtual RefObject {
public:
    explicit SyncAbleKvDBConnection(SyncAbleKvDB *kvDB);
    ~SyncAbleKvDBConnection() override;
    DISABLE_COPY_ASSIGN_MOVE(SyncAbleKvDBConnection);

    // Pragma interface.
    int Pragma(int cmd, void *parameter) override;

protected:
    int DisableManualSync();

    int EnableManualSync();

private:
    // Do pragma-sync action.
    int PragmaParamCheck(int cmd, const void *parameter);
    int PragmaSyncAction(const PragmaSync *syncParameter);

    // If enable is true, it will enable auto sync
    int EnableAutoSync(bool enable);

    void OnSyncComplete(const std::map<std::string, int> &statuses,
        const std::function<void(const std::map<std::string, int> &devicesMap)> &onComplete, bool wait);

    int GetQueuedSyncSize(int *queuedSyncSize) const;

    int SetQueuedSyncLimit(const int *queuedSyncLimit);

    int GetQueuedSyncLimit(int *queuedSyncLimit) const;

    int SetStaleDataWipePolicy(const WipePolicy *policy);

    int SetRemotePushFinishedNotify(PragmaRemotePushNotify *notifyParma);

    // For sync in progress.
    std::list<int> syncIdList_;

    std::mutex remotePushFinishedListenerLock_;
    NotificationChain::Listener *remotePushFinishedListener_;
};
}

#endif // SYNC_ABLE_KVDB_CONNECTION_H
