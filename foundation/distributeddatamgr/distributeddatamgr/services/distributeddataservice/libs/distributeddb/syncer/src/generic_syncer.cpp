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

#include "generic_syncer.h"

#include "db_errno.h"
#include "log_print.h"
#include "ref_object.h"
#include "sqlite_single_ver_natural_store.h"
#include "time_sync.h"
#include "single_ver_data_sync.h"
#ifndef OMIT_MULTI_VER
#include "commit_history_sync.h"
#include "multi_ver_data_sync.h"
#include "value_slice_sync.h"
#endif
#include "device_manager.h"
#include "db_constant.h"
#include "ability_sync.h"

namespace DistributedDB {
const int GenericSyncer::MIN_VALID_SYNC_ID = 1;
std::mutex GenericSyncer::moduleInitLock_;
int GenericSyncer::currentSyncId_ = 0;
std::mutex GenericSyncer::syncIdLock_;
GenericSyncer::GenericSyncer()
    : syncEngine_(nullptr),
      syncInterface_(nullptr),
      timeHelper_(nullptr),
      metadata_(nullptr),
      initialized_(false),
      queuedManualSyncSize_(0),
      queuedManualSyncLimit_(DBConstant::QUEUED_SYNC_LIMIT_DEFAULT),
      manualSyncEnable_(true),
      closing_(false)

{
}

GenericSyncer::~GenericSyncer()
{
    LOGD("[GenericSyncer] ~GenericSyncer!");
    if (syncEngine_ != nullptr) {
        RefObject::KillAndDecObjRef(syncEngine_);
        syncEngine_ = nullptr;
    }
    timeHelper_ = nullptr;
    metadata_ = nullptr;
    syncInterface_ = nullptr;
}

int GenericSyncer::Initialize(IKvDBSyncInterface *syncInterface)
{
    if (syncInterface == nullptr) {
        LOGE("[Syncer] Init failed, the syncInterface is null!");
        return -E_INVALID_ARGS;
    }

    {
        std::lock_guard<std::mutex> lock(syncerLock_);
        if (initialized_) {
            return E_OK;
        }
        if (closing_) {
            LOGE("[Syncer] Syncer is closing, return!");
            return -E_BUSY;
        }

        // As metadata_ will be used in EraseDeviceWaterMark, it should not be clear even if engine init failed.
        // It will be clear in destructor.
        int errCodeMetadata = InitMetaData(syncInterface);

        // As timeHelper_ will be used in GetTimeStamp, it should not be clear even if engine init failed.
        // It will be clear in destructor.
        int errCodeTimeHelper = InitTimeHelper(syncInterface);
        if (errCodeMetadata != E_OK || errCodeTimeHelper != E_OK) {
            return -E_INTERNAL_ERROR;
        }

        if (!RuntimeContext::GetInstance()->IsCommunicatorAggregatorValid()) {
            LOGW("[Syncer] Communicator component not ready!");
            return -E_NOT_INIT;
        }

        int errCode = SyncModuleInit();
        if (errCode != E_OK) {
            LOGE("[Syncer] Sync ModuleInit ERR!");
            return -E_INTERNAL_ERROR;
        }

        errCode = InitSyncEngine(syncInterface);
        if (errCode != E_OK) {
            return errCode;
        }

        initialized_ = true;
    }

    // RegConnectCallback may start a auto sync, this function can not in syncerLock_
    syncEngine_->RegConnectCallback();
    return E_OK;
}

int GenericSyncer::Close()
{
    {
        std::lock_guard<std::mutex> lock(syncerLock_);
        if (!initialized_) {
            LOGW("[Syncer] Syncer don't need to close, because it has no been init.");
            return -E_NOT_INIT;
        }
        initialized_ = false;
        if (closing_) {
            LOGE("[Syncer] Syncer is closing, return!");
            return -E_BUSY;
        }
        closing_ = true;
    }
    ClearSyncOperations();
    if (syncEngine_ != nullptr) {
        syncEngine_->OnKill([this]() { this->syncEngine_->Close(); });
        LOGD("[Syncer] Close SyncEngine!");
        RefObject::KillAndDecObjRef(syncEngine_);
        std::lock_guard<std::mutex> lock(syncerLock_);
        syncEngine_ = nullptr;
        closing_ = false;
    }
    timeHelper_ = nullptr;
    metadata_ = nullptr;
    return E_OK;
}

int GenericSyncer::Sync(const std::vector<std::string> &devices, int mode,
    const std::function<void(const std::map<std::string, int> &)> &onComplete,
    const std::function<void(void)> &onFinalize, bool wait = false)
{
    std::lock_guard<std::mutex> lock(syncerLock_);
    if (!initialized_) {
        LOGE("[Syncer] Syncer is not initialized, return!");
        return -E_NOT_INIT;
    }
    if (closing_) {
        LOGE("[Syncer] Syncer is closing, return!");
        return -E_BUSY;
    }
    if (!IsValidDevices(devices) || !IsValidMode(mode)) {
        return -E_INVALID_ARGS;
    }
    if (IsQueuedManualSyncFull(mode, wait)) {
        LOGE("[Syncer] -E_BUSY");
        return -E_BUSY;
    }
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(PT_TEST_RECORDS::RECORD_SYNC_TOTAL);
    }
    uint32_t syncId = GenerateSyncId();
    LOGI("[Syncer] GenerateSyncId %d, mode = %d, wait = %d , label = %s, devicesNum = %d", syncId, mode, wait,
        label_.c_str(), devices.size());
    SyncOperation *operation = new (std::nothrow) SyncOperation(syncId, devices, mode, onComplete, wait);
    if (operation == nullptr) {
        LOGE("[Syncer] SyncOperation alloc failed when sync called, may be out of memory");
        return -E_OUT_OF_MEMORY;
    }
    operation->Initialize();
    operation->OnKill(std::bind(&GenericSyncer::SyncOperationKillCallback, this, operation->GetSyncId()));
    std::function<void(int)> onFinished = std::bind(&GenericSyncer::OnSyncFinished, this, std::placeholders::_1);
    operation->SetOnSyncFinished(onFinished);
    operation->SetOnSyncFinalize(onFinalize);
    int errCode = AddSyncOperation(operation);
    if (errCode != E_OK) {
        LOGE("[Syncer] AddSyncOperation failed when sync called, err %d", errCode);
        RefObject::KillAndDecObjRef(operation);
        return errCode;
    }
    AddQueuedManualSyncSize(mode, wait);
    LOGD("[Syncer] AddSyncOperation end");
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(PT_TEST_RECORDS::RECORD_SYNC_TOTAL);
    }
    return syncId;
}

int GenericSyncer::RemoveSyncOperation(int syncId)
{
    SyncOperation *operation = nullptr;
    std::unique_lock<std::mutex> lock(operationMapLock_);
    auto iter = syncOperationMap_.find(syncId);
    if (iter != syncOperationMap_.end()) {
        LOGD("[Syncer] RemoveSyncOperation id:%d.", syncId);
        operation = iter->second;
        syncOperationMap_.erase(syncId);
        lock.unlock();
        if ((!operation->IsAutoSync()) && (!operation->IsBlockSync())) {
            SubQueuedSyncSize();
        }
        operation->NotifyIfNeed();
        RefObject::KillAndDecObjRef(operation);
        operation = nullptr;
        return E_OK;
    }
    return -E_INVALID_ARGS;
}

uint64_t GenericSyncer::GetTimeStamp()
{
    if (timeHelper_ == nullptr) {
        return TimeHelper::GetSysCurrentTime();
    }
    return timeHelper_->GetTime();
}

int GenericSyncer::AddSyncOperation(SyncOperation *operation)
{
    LOGD("[Syncer] AddSyncOperation.");
    if (operation == nullptr) {
        return -E_INVALID_ARGS;
    }

    int errCode = syncEngine_->AddSyncOperation(operation);
    if (errCode != E_OK) {
        return errCode;
    }

    if (operation->CheckIsAllFinished()) {
        if (operation->IsBlockSync()) {
            operation->Finished();
            RefObject::KillAndDecObjRef(operation);
            return errCode;
        }
        RefObject::IncObjRef(operation);
        RefObject::IncObjRef(syncEngine_);
        ISyncEngine *syncEngine = syncEngine_;
        errCode = RuntimeContext::GetInstance()->ScheduleTask([operation, syncEngine, this] {
            std::lock_guard<std::mutex> lock(syncerLock_);
            if (closing_) {
                LOGI("[Syncer] Syncer is closing, return!");
                RefObject::DecObjRef(operation);
                RefObject::DecObjRef(syncEngine);
                return;
            }
            operation->Finished();
            RefObject::KillAndDecObjRef(operation);
            RefObject::DecObjRef(operation);
            RefObject::DecObjRef(syncEngine);
        });
        if (errCode != E_OK) {
            LOGE("[Syncer] AddSyncOperation start finish task errCode:%d", errCode);
            RefObject::DecObjRef(operation);
            RefObject::DecObjRef(syncEngine_);
        }
        return errCode;
    }
    {
        std::lock_guard<std::mutex> lock(operationMapLock_);
        syncOperationMap_.insert(std::pair<int, SyncOperation *>(operation->GetSyncId(), operation));
        // To make sure operation alive before WaitIfNeed out
        RefObject::IncObjRef(operation);
    }
    operation->WaitIfNeed();
    RefObject::DecObjRef(operation);
    return errCode;
}

void GenericSyncer::SyncOperationKillCallbackInner(int syncId)
{
    if (syncEngine_ != nullptr) {
        LOGI("[Syncer] Operation on kill id = %d", syncId);
        syncEngine_->RemoveSyncOperation(syncId);
    }
}

void GenericSyncer::SyncOperationKillCallback(int syncId)
{
    SyncOperationKillCallbackInner(syncId);
}

int GenericSyncer::InitMetaData(IKvDBSyncInterface *syncInterface)
{
    if (metadata_ != nullptr) {
        return E_OK;
    }

    metadata_ = std::make_shared<Metadata>();
    int errCode = metadata_->Initialize(syncInterface);
    if (errCode != E_OK) {
        LOGE("[Syncer] metadata Initializeate failed! err %d.", errCode);
        metadata_ = nullptr;
    }
    return errCode;
}

int GenericSyncer::InitTimeHelper(IKvDBSyncInterface *syncInterface)
{
    if (timeHelper_ != nullptr) {
        return E_OK;
    }

    timeHelper_ = std::make_shared<TimeHelper>();
    int errCode = timeHelper_->Initialize(syncInterface, metadata_);
    if (errCode != E_OK) {
        LOGE("[Syncer] TimeHelper init failed! err:%d.", errCode);
        timeHelper_ = nullptr;
    }
    return errCode;
}

int GenericSyncer::InitSyncEngine(IKvDBSyncInterface *syncInterface)
{
    syncEngine_ = CreateSyncEngine();
    if (syncEngine_ == nullptr) {
        return -E_OUT_OF_MEMORY;
    }

    syncEngine_->OnLastRef([]() { LOGD("[Syncer] SyncEngine finalized"); });
    const std::function<void(std::string)> func = std::bind(&GenericSyncer::RemoteDataChanged,
        this, std::placeholders::_1);
    int errCode = syncEngine_->Initialize(syncInterface, metadata_, func);
    if (errCode == E_OK) {
        syncInterface_ = syncInterface;
        syncInterface->IncRefCount();
        label_ = syncEngine_->GetLabel();
        return E_OK;
    } else {
        LOGE("[Syncer] SyncEngine init failed! err:%d.", errCode);
        if (syncEngine_ != nullptr) {
            RefObject::KillAndDecObjRef(syncEngine_);
            syncEngine_ = nullptr;
        }
        return errCode;
    }
}

uint32_t GenericSyncer::GenerateSyncId()
{
    std::lock_guard<std::mutex> lock(syncIdLock_);
    currentSyncId_++;
    // if overflow, reset to 1
    if (currentSyncId_ <= 0) {
        currentSyncId_ = MIN_VALID_SYNC_ID;
    }
    return currentSyncId_;
}

bool GenericSyncer::IsValidMode(int mode) const
{
    if ((mode > SyncOperation::AUTO_PULL) || (mode < SyncOperation::PUSH)) {
        LOGE("[Syncer] Sync mode is not valid!");
        return false;
    }
    return true;
}

bool GenericSyncer::IsValidDevices(const std::vector<std::string> &devices) const
{
    if (devices.empty()) {
        LOGE("[Syncer] devices is empty!");
        return false;
    }
    return true;
}

void GenericSyncer::ClearSyncOperations()
{
    std::lock_guard<std::mutex> lock(operationMapLock_);
    for (auto &iter : syncOperationMap_) {
        RefObject::KillAndDecObjRef(iter.second);
        iter.second = nullptr;
    }
    syncOperationMap_.clear();
}

void GenericSyncer::OnSyncFinished(int syncId)
{
    (void)(RemoveSyncOperation(syncId));
}

int GenericSyncer::SyncModuleInit()
{
    static bool isInit = false;
    std::lock_guard<std::mutex> lock(moduleInitLock_);
    if (!isInit) {
        int errCode = SyncResourceInit();
        if (errCode != E_OK) {
            return errCode;
        }
        isInit = true;
        return E_OK;
    }
    return E_OK;
}

int GenericSyncer::SyncResourceInit()
{
    int errCode = TimeSync::RegisterTransformFunc();
    if (errCode != E_OK) {
        LOGE("Register timesync message transform func ERR!");
        return errCode;
    }
    errCode = SingleVerDataSync::RegisterTransformFunc();
    if (errCode != E_OK) {
        LOGE("Register SingleVerDataSync message transform func ERR!");
        return errCode;
    }
#ifndef OMIT_MULTI_VER
    errCode = CommitHistorySync::RegisterTransformFunc();
    if (errCode != E_OK) {
        LOGE("Register CommitHistorySync message transform func ERR!");
        return errCode;
    }
    errCode = MultiVerDataSync::RegisterTransformFunc();
    if (errCode != E_OK) {
        LOGE("Register MultiVerDataSync message transform func ERR!");
        return errCode;
    }
    errCode = ValueSliceSync::RegisterTransformFunc();
    if (errCode != E_OK) {
        LOGE("Register ValueSliceSync message transform func ERR!");
        return errCode;
    }
#endif
    errCode = DeviceManager::RegisterTransformFunc();
    if (errCode != E_OK) {
        LOGE("Register DeviceManager message transform func ERR!");
        return errCode;
    }
    errCode = AbilitySync::RegisterTransformFunc();
    if (errCode != E_OK) {
        LOGE("Register AbilitySync message transform func ERR!");
        return errCode;
    }
    return E_OK;
}

int GenericSyncer::GetQueuedSyncSize(int *queuedSyncSize) const
{
    if (queuedSyncSize == nullptr) {
        return -E_INVALID_ARGS;
    }
    std::lock_guard<std::mutex> lock(queuedManualSyncLock_);
    *queuedSyncSize = queuedManualSyncSize_;
    LOGI("[GenericSyncer] GetQueuedSyncSize:%d", queuedManualSyncSize_);
    return E_OK;
}

int GenericSyncer::SetQueuedSyncLimit(const int *queuedSyncLimit)
{
    if (queuedSyncLimit == nullptr) {
        return -E_INVALID_ARGS;
    }
    std::lock_guard<std::mutex> lock(queuedManualSyncLock_);
    queuedManualSyncLimit_ = *queuedSyncLimit;
    LOGI("[GenericSyncer] SetQueuedSyncLimit:%d", queuedManualSyncLimit_);
    return E_OK;
}

int GenericSyncer::GetQueuedSyncLimit(int *queuedSyncLimit) const
{
    if (queuedSyncLimit == nullptr) {
        return -E_INVALID_ARGS;
    }
    std::lock_guard<std::mutex> lock(queuedManualSyncLock_);
    *queuedSyncLimit = queuedManualSyncLimit_;
    LOGI("[GenericSyncer] GetQueuedSyncLimit:%d", queuedManualSyncLimit_);
    return E_OK;
}

void GenericSyncer::AddQueuedManualSyncSize(int mode, bool wait)
{
    if (((mode == SyncOperation::PULL) || (mode == SyncOperation::PUSH) || (mode == SyncOperation::PUSH_AND_PULL)) &&
        (wait == false)) {
        std::lock_guard<std::mutex> lock(queuedManualSyncLock_);
        queuedManualSyncSize_++;
    }
}

bool GenericSyncer::IsQueuedManualSyncFull(int mode, bool wait) const
{
    std::lock_guard<std::mutex> lock(queuedManualSyncLock_);
    if (((mode == SyncOperation::PULL) || (mode == SyncOperation::PUSH) || (mode == SyncOperation::PUSH_AND_PULL)) &&
        (manualSyncEnable_ == false)) {
        LOGI("[GenericSyncer] manualSyncEnable_:false");
        return true;
    }
    if (((mode == SyncOperation::PULL) || (mode == SyncOperation::PUSH) || (mode == SyncOperation::PUSH_AND_PULL)) &&
        (wait == false)) {
        if (queuedManualSyncSize_ < queuedManualSyncLimit_) {
            return false;
        }
        LOGD("[GenericSyncer] queuedManualSyncSize_:%d < queuedManualSyncLimit_:%d", queuedManualSyncSize_,
            queuedManualSyncLimit_);
        return true;
    }
    return false;
}

void GenericSyncer::SubQueuedSyncSize(void)
{
    std::lock_guard<std::mutex> lock(queuedManualSyncLock_);
    queuedManualSyncSize_--;
    if (queuedManualSyncSize_ < 0) {
        LOGE("[GenericSyncer] queuedManualSyncSize_ < 0!");
        queuedManualSyncSize_ = 0;
    }
}

int GenericSyncer::DisableManualSync(void)
{
    std::lock_guard<std::mutex> lock(queuedManualSyncLock_);
    if (queuedManualSyncSize_ > 0) {
        LOGD("[GenericSyncer] DisableManualSync fail, queuedManualSyncSize_:%d", queuedManualSyncSize_);
        return -E_BUSY;
    }
    manualSyncEnable_ = false;
    LOGD("[GenericSyncer] DisableManualSync ok");
    return E_OK;
}

int GenericSyncer::EnableManualSync(void)
{
    std::lock_guard<std::mutex> lock(queuedManualSyncLock_);
    manualSyncEnable_ = true;
    LOGD("[GenericSyncer] EnableManualSync ok");
    return E_OK;
}

int GenericSyncer::GetLocalIdentity(std::string &outTarget) const
{
    std::lock_guard<std::mutex> lock(syncerLock_);
    if (!initialized_) {
        LOGE("[Syncer] Syncer is not initialized, return!");
        return -E_NOT_INIT;
    }
    if (closing_) {
        LOGE("[Syncer] Syncer is closing, return!");
        return -E_BUSY;
    }
    if (syncEngine_ == nullptr) {
        LOGE("[Syncer] Syncer syncEngine_ is nullptr, return!");
        return -E_NOT_INIT;
    }
    return syncEngine_->GetLocalIdentity(outTarget);
}

void GenericSyncer::GetOnlineDevices(std::vector<std::string> &devices) const
{
    // Get devices from AutoLaunch first.
    if (syncInterface_ == nullptr) {
        LOGI("[Syncer] GetOnlineDevices syncInterface_ is nullptr");
        return;
    }
    std::string identifier = syncInterface_->GetDbProperties().GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    // If this database is configured as autoLaunch, then this database on other devices is probably autoLaunch as well.
    // Since this database on other devices might have not been opened, we have to use physical online devices as list.
    // If this database is not configured as autoLaunch, the out devices will be empty.
    RuntimeContext::GetInstance()->GetAutoLaunchSyncDevices(identifier, devices);
    if (!devices.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(syncerLock_);
    if (closing_) {
        LOGE("[Syncer] Syncer is closing, return!");
        return;
    }
    if (syncEngine_ != nullptr) {
        syncEngine_->GetOnlineDevices(devices);
    }
}
} // namespace DistributedDB
