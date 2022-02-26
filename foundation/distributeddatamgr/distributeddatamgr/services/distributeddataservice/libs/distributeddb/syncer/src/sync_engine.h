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

#ifndef SYNC_ENGINE_H
#define SYNC_ENGINE_H

#include <map>
#include <mutex>
#include <queue>

#include "isync_engine.h"
#include "isync_task_context.h"
#include "task_pool.h"
#include "device_manager.h"

namespace DistributedDB {
constexpr uint16_t NEW_SEND_TASK = 1;

class SyncEngine : public ISyncEngine {
public:
    SyncEngine();
    virtual ~SyncEngine();

    // Do some init things
    int Initialize(IKvDBSyncInterface *syncInterface, std::shared_ptr<Metadata> &metadata,
        const std::function<void(std::string)> &onRemoteDataChanged) override;

    // Do some things, when db close.
    int Close() override;

    // Alloc and Add sync SyncTarget
    // return E_OK if operator success.
    int AddSyncOperation(SyncOperation *operation) override;

    // Clear the SyncTarget matched the syncId.
    void RemoveSyncOperation(int syncId) override;

    // notify other devices data has changed
    void BroadCastDataChanged() const override;

    // Get Online devices
    void GetOnlineDevices(std::vector<std::string> &devices) const override;

    // Register the device connect callback, this function must be called after Engine inited
    void RegConnectCallback() override;

    // Get the queue cache memory size
    int GetQueueCacheSize() const;

    // Get the number of message which is discarded
    unsigned int GetDiscardMsgNum() const;

    // Get the maximum of executing message number
    unsigned int GetMaxExecNum() const;

    // Set the maximum of queue cache memory size
    void SetMaxQueueCacheSize(int value);

    // Get local deviceId, is hashed
    int GetLocalIdentity(std::string &outTarget) const override;

    std::string GetLabel() const override;

protected:
    // Create a context
    virtual ISyncTaskContext *CreateSyncTaskContext() = 0;

    // Find SyncTaskContext from the map
    ISyncTaskContext *FindSyncTaskContext(const std::string &deviceId);

    // Used to store all send sync task infos (such as pull sync response, and push sync request)
    std::map<std::string, ISyncTaskContext *> syncTaskContextMap_;
    std::mutex contextMapLock_;

private:

    // Init DeviceManager set callback
    int InitDeviceManager(const std::function<void(std::string)> &onRemoteDataChanged);

    ISyncTaskContext *GetSyncTaskContext(const std::string &deviceId, int &errCode);

    // Init Comunicator, register callbacks
    int InitComunicator(const IKvDBSyncInterface *syncInterface);

    // Add the sync task info to the map.
    int AddSyncOperForContext(const std::string &deviceId, SyncOperation *operation);

    // Sync Request CallbackTask run at a sub thread.
    void MessageReceiveCallbackTask(ISyncTaskContext *context, const ICommunicator *communicator, Message *inMsg);

    // wrapper of MessageReceiveCallbackTask
    void MessageReceiveCallback(const std::string &targetDev, Message *inMsg);

    // Sync Request Callback
    int MessageReceiveCallbackInner(const std::string &targetDev, Message *inMsg);

    // Exec the given SyncTarget. and callback onComplete.
    int ExecSyncTask(ISyncTaskContext *context);

    // Anti-DOS attack
    void PutMsgIntoQueue(const std::string &targetDev, Message *inMsg, int msgSize);

    // Get message size
    int GetMsgSize(const Message *inMsg) const;

    // Do not run MessageReceiveCallbackTask until msgQueue is empty
    int DealMsgUtilQueueEmpty();

    // Handle message in order.
    int ScheduleDealMsg(ISyncTaskContext *context, Message *inMsg);

    // Schedule Sync Task
    void ScheduleSyncTask(ISyncTaskContext *context);

    ISyncTaskContext *GetConextForMsg(const std::string &targetDev, int &errCode);

    int RunPermissionCheck(const std::string &deviceId, int mode) const;

    IKvDBSyncInterface *syncInterface_;
    ICommunicator *communicator_;
    DeviceManager *deviceManager_;
    std::function<void(const std::string &)> onRemoteDataChanged_;
    std::shared_ptr<Metadata> metadata_;
    std::deque<Message *> msgQueue_;
    NotificationChain::Listener *timeChangedListener_;
    unsigned int execTaskCount_;
    std::string label_;

    static int queueCacheSize_;
    static int maxQueueCacheSize_;
    static unsigned int discardMsgNum_;
    static const unsigned int MAX_EXEC_NUM = 7; // Set the maximum of threads as 6 < 7
    static constexpr int DEFAULT_CACHE_SIZE = 160 * 1024 * 1024; // Initial the default cache size of queue as 160MB
    static std::mutex queueLock_;
};
} // namespace DistributedDB

#endif // SYNC_ENGINE_H
