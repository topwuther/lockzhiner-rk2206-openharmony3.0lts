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

#include "sync_able_kvdb.h"
#include "db_errno.h"
#include "log_print.h"
#include "parcel.h"

namespace DistributedDB {
const EventType SyncAbleKvDB::REMOTE_PUSH_FINISHED = 1;

SyncAbleKvDB::SyncAbleKvDB()
    : started_(false),
      remotePushNotifyChain_(nullptr)
{}

SyncAbleKvDB::~SyncAbleKvDB()
{
    if (remotePushNotifyChain_ != nullptr) {
        (void)remotePushNotifyChain_->UnRegisterEventType(REMOTE_PUSH_FINISHED);
        KillAndDecObjRef(remotePushNotifyChain_);
        remotePushNotifyChain_ = nullptr;
    }
}

void SyncAbleKvDB::DelConnection(GenericKvDBConnection *connection)
{
    auto realConnection = static_cast<SyncAbleKvDBConnection *>(connection);
    if (realConnection != nullptr) {
        KillAndDecObjRef(realConnection);
        realConnection = nullptr;
    }
}

void SyncAbleKvDB::TriggerSync(int notifyEvent)
{
    if (!started_) {
        StartSyncer();
    }
    if (started_) {
        syncer_.LocalDataChanged(notifyEvent);
    }
}

void SyncAbleKvDB::CommitNotify(int notifyEvent, KvDBCommitNotifyFilterAbleData *data)
{
    SyncAbleKvDB::TriggerSync(notifyEvent);

    GenericKvDB::CommitNotify(notifyEvent, data);
}

void SyncAbleKvDB::Close()
{
    StopSyncer();
}

// Start a sync action.
int SyncAbleKvDB::Sync(const std::vector<std::string> &devices, int mode,
    const std::function<void(const std::map<std::string, int> &)> &onComplete,
    const std::function<void(void)> &onFinalize, bool wait = false)
{
    if (!started_) {
        StartSyncer();
        if (!started_) {
            return -E_NOT_INIT;
        }
    }
    return syncer_.Sync(devices, mode, onComplete, onFinalize, wait);
}

void SyncAbleKvDB::EnableAutoSync(bool enable)
{
    if (!started_) {
        StartSyncer();
    }
    return syncer_.EnableAutoSync(enable);
}

void SyncAbleKvDB::WakeUpSyncer()
{
    StartSyncer();
}

// Stop a sync action in progress.
void SyncAbleKvDB::StopSync(int syncId)
{
    if (started_) {
        syncer_.RemoveSyncOperation(syncId);
    }
}

// Start syncer
void SyncAbleKvDB::StartSyncer()
{
    IKvDBSyncInterface *syncInterface = GetSyncInterface();
    if (syncInterface == nullptr) {
        LOGF("KvDB got null sync interface.");
        return;
    }

    int errCode = syncer_.Initialize(syncInterface);
    if (errCode == E_OK) {
        started_ = true;
    } else {
        LOGE("KvDB start syncer failed, err:'%d'.", errCode);
    }
}

// Stop syncer
void SyncAbleKvDB::StopSyncer()
{
    if (started_) {
        syncer_.Close();
        started_ = false;
    }
}

// Get The current virtual timestamp
uint64_t SyncAbleKvDB::GetTimeStamp()
{
    if (!started_) {
        StartSyncer();
    }
    return syncer_.GetTimeStamp();
}

// Get the dataItem's append length
uint32_t SyncAbleKvDB::GetAppendedLen() const
{
    return Parcel::GetAppendedLen();
}

int SyncAbleKvDB::EraseDeviceWaterMark(const std::string &deviceId, bool isNeedHash)
{
    if (!started_) {
        StartSyncer();
    }
    return syncer_.EraseDeviceWaterMark(deviceId, isNeedHash);
}

int SyncAbleKvDB::GetQueuedSyncSize(int *queuedSyncSize) const
{
    return syncer_.GetQueuedSyncSize(queuedSyncSize);
}

int SyncAbleKvDB::SetQueuedSyncLimit(const int *queuedSyncLimit)
{
    return syncer_.SetQueuedSyncLimit(queuedSyncLimit);
}

int SyncAbleKvDB::GetQueuedSyncLimit(int *queuedSyncLimit) const
{
    return syncer_.GetQueuedSyncLimit(queuedSyncLimit);
}

int SyncAbleKvDB::DisableManualSync(void)
{
    return syncer_.DisableManualSync();
}

int SyncAbleKvDB::EnableManualSync(void)
{
    return syncer_.EnableManualSync();
}

int SyncAbleKvDB::GetLocalIdentity(std::string &outTarget)
{
    if (!started_) {
        StartSyncer();
    }
    return syncer_.GetLocalIdentity(outTarget);
}

int SyncAbleKvDB::SetStaleDataWipePolicy(WipePolicy policy)
{
    if (!started_) {
        StartSyncer();
    }
    return syncer_.SetStaleDataWipePolicy(policy);
}

NotificationChain::Listener *SyncAbleKvDB::AddRemotePushFinishedNotify(const RemotePushFinishedNotifier &notifier,
    int errCode)
{
    {
        std::lock_guard<std::mutex> lock(remotePushNotifyChainLock_);
        if (remotePushNotifyChain_ == nullptr) {
            remotePushNotifyChain_ = new (std::nothrow) NotificationChain;
            if (remotePushNotifyChain_ == nullptr) {
                errCode = -E_OUT_OF_MEMORY;
                return nullptr;
            }

            errCode = remotePushNotifyChain_->RegisterEventType(REMOTE_PUSH_FINISHED);
            if (errCode != E_OK) {
                LOGE("[SyncAbleKvDB] Register remote push finished event type failed! err %d", errCode);
                KillAndDecObjRef(remotePushNotifyChain_);
                remotePushNotifyChain_ = nullptr;
                return nullptr;
            }
        }
    }

    NotificationChain::Listener *listener =
        remotePushNotifyChain_->RegisterListener(REMOTE_PUSH_FINISHED,
            [notifier](void *arg) {
                notifier(*static_cast<RemotePushNotifyInfo *>(arg));
            }, nullptr, errCode);
    if (errCode != E_OK) {
        LOGE("[SyncAbleKvDB] Add remote push finished notifier failed! err %d", errCode);
    }
    return listener;
}

void SyncAbleKvDB::NotifyRemotePushFinishedInner(const std::string &targetId) const
{
    if (remotePushNotifyChain_ != nullptr) {
        RemotePushNotifyInfo info;
        info.deviceId = targetId;
        remotePushNotifyChain_->NotifyEvent(REMOTE_PUSH_FINISHED, static_cast<void *>(&info));
    }
}
}
