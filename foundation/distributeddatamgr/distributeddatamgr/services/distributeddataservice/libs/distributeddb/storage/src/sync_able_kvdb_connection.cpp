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

#include "sync_able_kvdb_connection.h"

#include "log_print.h"
#include "db_errno.h"
#include "db_constant.h"
#include "kvdb_pragma.h"
#include "performance_analysis.h"
#include "sync_able_kvdb.h"

namespace DistributedDB {
SyncAbleKvDBConnection::SyncAbleKvDBConnection(SyncAbleKvDB *kvDB)
    : GenericKvDBConnection(kvDB),
      remotePushFinishedListener_(nullptr)
{
    OnKill([this]() {
        for (const auto &syncId : syncIdList_) {
            SyncAbleKvDB *db = GetDB<SyncAbleKvDB>();
            if (syncId <= 0 || db == nullptr) {
                continue;
            }

            // Drop the lock before we call RemoveSyncOperation().
            UnlockObj();
            db->StopSync(syncId);
            LockObj();
        }
        syncIdList_.clear();
    });
}

SyncAbleKvDBConnection::~SyncAbleKvDBConnection()
{
    if (remotePushFinishedListener_ != nullptr) {
        remotePushFinishedListener_->Drop(true);
    }
    remotePushFinishedListener_ = nullptr;
}

int SyncAbleKvDBConnection::Pragma(int cmd, void *parameter)
{
    int errCode = PragmaParamCheck(cmd, parameter);
    if (errCode != E_OK) {
        return -E_INVALID_ARGS;
    }
    switch (cmd) {
        case PRAGMA_SYNC_DEVICES:
            errCode = PragmaSyncAction(static_cast<PragmaSync *>(parameter));
            break;
        case PRAGMA_AUTO_SYNC:
            errCode = EnableAutoSync(*(static_cast<bool *>(parameter)));
            break;
        case PRAGMA_PERFORMANCE_ANALYSIS_GET_REPORT:
            *(static_cast<std::string *>(parameter)) = PerformanceAnalysis::GetInstance()->GetStatistics();
            break;
        case PRAGMA_PERFORMANCE_ANALYSIS_OPEN:
            PerformanceAnalysis::GetInstance()->OpenPerformanceAnalysis();
            break;
        case PRAGMA_PERFORMANCE_ANALYSIS_CLOSE:
            PerformanceAnalysis::GetInstance()->ClosePerformanceAnalysis();
            break;
        case PRAGMA_PERFORMANCE_ANALYSIS_SET_REPORTFILENAME:
            PerformanceAnalysis::GetInstance()->SetFileNumber(*(static_cast<std::string *>(parameter)));
            break;
        case PRAGMA_GET_QUEUED_SYNC_SIZE:
            errCode = GetQueuedSyncSize(static_cast<int *>(parameter));
            break;
        case PRAGMA_SET_QUEUED_SYNC_LIMIT:
            errCode = SetQueuedSyncLimit(static_cast<int *>(parameter));
            break;
        case PRAGMA_GET_QUEUED_SYNC_LIMIT:
            errCode = GetQueuedSyncLimit(static_cast<int *>(parameter));
            break;
        case PRAGMA_SET_WIPE_POLICY:
            errCode = SetStaleDataWipePolicy(static_cast<WipePolicy *>(parameter));
            break;
        case PRAGMA_REMOTE_PUSH_FINISHED_NOTIFY:
            errCode = SetRemotePushFinishedNotify(static_cast<PragmaRemotePushNotify *>(parameter));
            break;
        default:
            // Call Pragma() of super class.
            errCode = GenericKvDBConnection::Pragma(cmd, parameter);
            break;
    }
    return errCode;
}

int SyncAbleKvDBConnection::PragmaParamCheck(int cmd, const void *parameter)
{
    switch (cmd) {
        case PRAGMA_AUTO_SYNC:
        case PRAGMA_PERFORMANCE_ANALYSIS_GET_REPORT:
        case PRAGMA_PERFORMANCE_ANALYSIS_SET_REPORTFILENAME:
            if (parameter == nullptr) {
                return -E_INVALID_ARGS;
            }
            return E_OK;
        default:
            return E_OK;
    }
}

int SyncAbleKvDBConnection::PragmaSyncAction(const PragmaSync *syncParameter)
{
    if (syncParameter == nullptr) {
        return -E_INVALID_ARGS;
    }
    SyncAbleKvDB *kvDB = GetDB<SyncAbleKvDB>();
    if (kvDB == nullptr) {
        return -E_INVALID_CONNECTION;
    }

    if (isExclusive_.load()) {
        return -E_BUSY;
    }
    {
        AutoLock lockGuard(this);
        if (IsKilled()) {
            // If this happens, user are using a closed connection.
            LOGE("Pragma sync on a closed connection.");
            return -E_STALE;
        }
        IncObjRef(this);
    }
    int errCode = kvDB->Sync(syncParameter->devices_, syncParameter->mode_,
        std::bind(&SyncAbleKvDBConnection::OnSyncComplete, this, std::placeholders::_1, syncParameter->onComplete_,
            syncParameter->wait_), [this]() {
                DecObjRef(this);
            }, syncParameter->wait_);
    if (errCode <= 0) {
        DecObjRef(this);
    } else if (!syncParameter->wait_) {
        AutoLock lockGuard(this);
        if (!IsKilled()) {
            syncIdList_.push_back(errCode);
        }
    }
    return errCode;
}

int SyncAbleKvDBConnection::EnableAutoSync(bool enable)
{
    SyncAbleKvDB *kvDB = GetDB<SyncAbleKvDB>();
    if (kvDB == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    kvDB->EnableAutoSync(enable);
    return E_OK;
}

void SyncAbleKvDBConnection::OnSyncComplete(const std::map<std::string, int> &statuses,
    const std::function<void(const std::map<std::string, int> &devicesMap)> &onComplete, bool wait)
{
    AutoLock lockGuard(this);
    if (!wait && !IsKilled()) {
        if (!syncIdList_.empty()) {
            syncIdList_.pop_front();
        }
    }
    if (!IsKilled() && onComplete) {
        // Drop the lock before invoking the callback.
        // Do pragma-sync again in the prev sync callback is supported.
        UnlockObj();
        // The connection may be closed after UnlockObj().
        // RACE: 'KillObj()' against 'onComplete()'.
        if (!IsKilled()) {
            onComplete(statuses);
        }
        LockObj();
    }
}

int SyncAbleKvDBConnection::GetQueuedSyncSize(int *queuedSyncSize) const
{
    if (queuedSyncSize == nullptr) {
        return -E_INVALID_ARGS;
    }
    SyncAbleKvDB *kvDB = GetDB<SyncAbleKvDB>();
    if (kvDB == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    return kvDB->GetQueuedSyncSize(queuedSyncSize);
}

int SyncAbleKvDBConnection::SetQueuedSyncLimit(const int *queuedSyncLimit)
{
    if (queuedSyncLimit == nullptr) {
        return -E_INVALID_ARGS;
    }
    if ((*queuedSyncLimit > DBConstant::QUEUED_SYNC_LIMIT_MAX) ||
        (*queuedSyncLimit < DBConstant::QUEUED_SYNC_LIMIT_MIN)) {
        return -E_INVALID_ARGS;
    }
    SyncAbleKvDB *kvDB = GetDB<SyncAbleKvDB>();
    if (kvDB == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    return kvDB->SetQueuedSyncLimit(queuedSyncLimit);
}

int SyncAbleKvDBConnection::GetQueuedSyncLimit(int *queuedSyncLimit) const
{
    if (queuedSyncLimit == nullptr) {
        return -E_INVALID_ARGS;
    }
    SyncAbleKvDB *kvDB = GetDB<SyncAbleKvDB>();
    if (kvDB == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    return kvDB->GetQueuedSyncLimit(queuedSyncLimit);
}

int SyncAbleKvDBConnection::DisableManualSync(void)
{
    SyncAbleKvDB *kvDB = GetDB<SyncAbleKvDB>();
    if (kvDB == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    return kvDB->DisableManualSync();
}

int SyncAbleKvDBConnection::EnableManualSync(void)
{
    SyncAbleKvDB *kvDB = GetDB<SyncAbleKvDB>();
    if (kvDB == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    return kvDB->EnableManualSync();
}

int SyncAbleKvDBConnection::SetStaleDataWipePolicy(const WipePolicy *policy)
{
    if (policy == nullptr) {
        return -E_INVALID_ARGS;
    }
    SyncAbleKvDB *kvDB = GetDB<SyncAbleKvDB>();
    if (kvDB == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    return kvDB->SetStaleDataWipePolicy(*policy);
}

int SyncAbleKvDBConnection::SetRemotePushFinishedNotify(PragmaRemotePushNotify *notifyParma)
{
    if (notifyParma == nullptr) {
        return -E_INVALID_ARGS;
    }

    SyncAbleKvDB *kvDB = GetDB<SyncAbleKvDB>();
    if (kvDB == nullptr) {
        return -E_INVALID_CONNECTION;
    }

    int errCode = E_OK;
    NotificationChain::Listener *tmpListener = nullptr;
    if (notifyParma->notifier_ != nullptr) {
        tmpListener = kvDB->AddRemotePushFinishedNotify(notifyParma->notifier_, errCode);
        if (tmpListener == nullptr) {
            return errCode;
        }
    }

    std::lock_guard<std::mutex> lock(remotePushFinishedListenerLock_);
    // Drop old listener and set the new listener
    if (remotePushFinishedListener_ != nullptr) {
        errCode = remotePushFinishedListener_->Drop();
        if (errCode != E_OK) {
            LOGE("[SyncAbleConnection] Drop Remote push finished listener failed %d", errCode);
            if (tmpListener != nullptr) {
                tmpListener->Drop();
            }
            return errCode;
        }
    }
    remotePushFinishedListener_ = tmpListener;
    return errCode;
}
}
