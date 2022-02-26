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

#include "network_adapter.h"
#include "db_errno.h"
#include "log_print.h"
#include "runtime_context.h"

namespace DistributedDB {
namespace {
const std::string DEFAULT_PROCESS_LABEL = "Distributeddb_Anonymous_Process";
const std::string SCHEDULE_QUEUE_TAG = "NetworkAdapter";
constexpr uint32_t MIN_MTU_SIZE = 1024; // 1KB
constexpr uint32_t MAX_MTU_SIZE = 5242880; // 5MB
}

NetworkAdapter::NetworkAdapter()
    : processLabel_(DEFAULT_PROCESS_LABEL), processCommunicator_(nullptr)
{
}

NetworkAdapter::NetworkAdapter(const std::string &inProcessLabel)
    : processLabel_(inProcessLabel), processCommunicator_(nullptr)
{
}

NetworkAdapter::NetworkAdapter(const std::string &inProcessLabel,
    const std::shared_ptr<IProcessCommunicator> &inCommunicator)
    : processLabel_(inProcessLabel), processCommunicator_(inCommunicator)
{
}

NetworkAdapter::~NetworkAdapter()
{
}

int NetworkAdapter::StartAdapter()
{
    LOGI("[NAdapt][Start] Enter, ProcessLabel=%s.", processLabel_.c_str());
    if (processLabel_.empty()) {
        return -E_INVALID_ARGS;
    }
    if (!processCommunicator_) {
        LOGE("[NAdapt][Start] ProcessCommunicator not be designated yet.");
        return -E_INVALID_ARGS;
    }
    DBStatus errCode = processCommunicator_->Start(processLabel_);
    if (errCode != DBStatus::OK) {
        LOGE("[NAdapt][Start] Start Fail, errCode=%d.", static_cast<int>(errCode));
        return -E_PERIPHERAL_INTERFACE_FAIL;
    }
    errCode = processCommunicator_->RegOnDataReceive(std::bind(&NetworkAdapter::OnDataReceiveHandler, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    if (errCode != DBStatus::OK) {
        LOGE("[NAdapt][Start] RegOnDataReceive Fail, errCode=%d.", static_cast<int>(errCode));
        // DO ROLLBACK
        errCode = processCommunicator_->Stop();
        LOGI("[NAdapt][Start] ROLLBACK: Stop errCode=%d.", static_cast<int>(errCode));
        return -E_PERIPHERAL_INTERFACE_FAIL;
    }
    errCode = processCommunicator_->RegOnDeviceChange(std::bind(&NetworkAdapter::OnDeviceChangeHandler, this,
        std::placeholders::_1, std::placeholders::_2));
    if (errCode != DBStatus::OK) {
        LOGE("[NAdapt][Start] RegOnDeviceChange Fail, errCode=%d.", static_cast<int>(errCode));
        // DO ROLLBACK
        errCode = processCommunicator_->RegOnDataReceive(nullptr);
        LOGI("[NAdapt][Start] ROLLBACK: UnRegOnDataReceive errCode=%d.", static_cast<int>(errCode));
        errCode = processCommunicator_->Stop();
        LOGI("[NAdapt][Start] ROLLBACK: Stop errCode=%d.", static_cast<int>(errCode));
        return -E_PERIPHERAL_INTERFACE_FAIL;
    }
    // These code is compensation for the probable defect of IProcessCommunicator implementation.
    // As described in the agreement, for the missed online situation, we search for the online devices at beginning.
    // OnDeviceChangeHandler is reused to check the existence of peer process.
    // Since at this point, the CommunicatorAggregator had not been fully initialized,
    // We need an async task which bring about dependency on the lifecycle of this NetworkAdapter Object.
    SearchOnlineRemoteDeviceAtStartup();
    LOGI("[NAdapt][Start] Exit.");
    return E_OK;
}

// StartAdapter and StopAdapter are all innerly called by ICommunicatorAggregator
// If StopAdapter is called, the StartAdapter must have been called successfully before,
// so processCommunicator_ won't be null
void NetworkAdapter::StopAdapter()
{
    LOGI("[NAdapt][Stop] Enter, ProcessLabel=%s.", processLabel_.c_str());
    DBStatus errCode = processCommunicator_->RegOnDeviceChange(nullptr);
    if (errCode != DBStatus::OK) {
        LOGE("[NAdapt][Stop] UnRegOnDeviceChange Fail, errCode=%d.", static_cast<int>(errCode));
    }
    errCode = processCommunicator_->RegOnDataReceive(nullptr);
    if (errCode != DBStatus::OK) {
        LOGE("[NAdapt][Stop] UnRegOnDataReceive Fail, errCode=%d.", static_cast<int>(errCode));
    }
    errCode = processCommunicator_->Stop();
    if (errCode != DBStatus::OK) {
        LOGE("[NAdapt][Stop] Stop Fail, errCode=%d.", static_cast<int>(errCode));
    }
    // We don't reset the shared_ptr of commProvider here, the release of commProvider is done by deconstruct of adapter
    // In this way, the adapter can be start again after stop it, since it still hold the an valid commProvider
    // The async task is dependent on this Object. we have to wait until all async task finished.
    LOGI("[NAdapt][Stop] Wait all async task done.");
    std::unique_lock<std::mutex> asyncTaskDoneLock(asyncTaskDoneMutex_);
    asyncTaskDoneCv_.wait(asyncTaskDoneLock, [this]{ return pendingAsyncTaskCount_ <= 0; });
    LOGI("[NAdapt][Stop] Exit.");
}

namespace {
uint32_t CheckAndAdjustMtuSize(uint32_t inMtuSize)
{
    if (inMtuSize < MIN_MTU_SIZE) {
        return MIN_MTU_SIZE;
    } else if (inMtuSize > MAX_MTU_SIZE) {
        return MAX_MTU_SIZE;
    } else {
        return (inMtuSize - (inMtuSize % sizeof(uint64_t))); // Octet alignment
    }
}
}

uint32_t NetworkAdapter::GetMtuSize()
{
    std::lock_guard<std::mutex> mtuSizeLockGuard(mtuSizeMutex_);
    if (!isMtuSizeValid_) {
        mtuSize_ = processCommunicator_->GetMtuSize();
        LOGI("[NAdapt][GetMtu] mtuSize=%u.", mtuSize_);
        mtuSize_ = CheckAndAdjustMtuSize(mtuSize_);
        isMtuSizeValid_ = true;
    }
    return mtuSize_;
}

uint32_t NetworkAdapter::GetMtuSize(const std::string &target)
{
    std::lock_guard<std::mutex> mtuSizeLockGuard(mtuSizeMutex_);
    if (devMapMtuSize_.count(target) == 0) {
        DeviceInfos devInfo;
        devInfo.identifier = target;
        uint32_t oriMtuSize = processCommunicator_->GetMtuSize(devInfo);
        LOGI("[NAdapt][GetMtu] mtuSize=%u of target=%s{private}.", oriMtuSize, target.c_str());
        devMapMtuSize_[target] = CheckAndAdjustMtuSize(oriMtuSize);
    }
    return devMapMtuSize_[target];
}

int NetworkAdapter::GetLocalIdentity(std::string &outTarget)
{
    std::lock_guard<std::mutex> identityLockGuard(identityMutex_);
    if (!isLocalIdentityValid_) {
        DeviceInfos devInfo = processCommunicator_->GetLocalDeviceInfos();
        LOGI("[NAdapt][GetLocal] localIdentity=%s{private}.", devInfo.identifier.c_str());
        if (devInfo.identifier.empty()) {
            return -E_PERIPHERAL_INTERFACE_FAIL;
        }
        localIdentity_ = devInfo.identifier;
        isLocalIdentityValid_ = true;
    }
    outTarget = localIdentity_;
    return E_OK;
}

int NetworkAdapter::SendBytes(const std::string &dstTarget, const uint8_t *bytes, uint32_t length)
{
    if (bytes == nullptr || length == 0) {
        return -E_INVALID_ARGS;
    }
    LOGI("[NAdapt][SendBytes] Enter, to=%s{private}, length=%d", dstTarget.c_str(), length);
    DeviceInfos dstDevInfo;
    dstDevInfo.identifier = dstTarget;
    DBStatus errCode = processCommunicator_->SendData(dstDevInfo, bytes, length);
    if (errCode != DBStatus::OK) {
        LOGE("[NAdapt][SendBytes] SendData Fail, errCode=%d.", static_cast<int>(errCode));
        // These code is compensation for the probable defect of IProcessCommunicator implementation.
        // As described in the agreement, for the missed offline situation, we check if still online at send fail.
        // OnDeviceChangeHandler is reused but check the existence of peer process is done outerly.
        // Since this thread is the sending_thread of the CommunicatorAggregator,
        // We need an async task which bring about dependency on the lifecycle of this NetworkAdapter Object.
        CheckDeviceOfflineAfterSendFail(dstDevInfo);
        return -E_PERIPHERAL_INTERFACE_FAIL;
    }
    return E_OK;
}

int NetworkAdapter::RegBytesReceiveCallback(const BytesReceiveCallback &onReceive, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> onReceiveLockGard(onReceiveMutex_);
    return RegCallBack(onReceive, onReceiveHandle_, inOper, onReceiveFinalizer_);
}

int NetworkAdapter::RegTargetChangeCallback(const TargetChangeCallback &onChange, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> onChangeLockGard(onChangeMutex_);
    return RegCallBack(onChange, onChangeHandle_, inOper, onChangeFinalizer_);
}

int NetworkAdapter::RegSendableCallback(const SendableCallback &onSendable, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> onSendableLockGard(onSendableMutex_);
    return RegCallBack(onSendable, onSendableHandle_, inOper, onSendableFinalizer_);
}

void NetworkAdapter::OnDataReceiveHandler(const DeviceInfos &srcDevInfo, const uint8_t *data, uint32_t length)
{
    if (data == nullptr || length == 0) {
        LOGE("[NAdapt][OnDataRecv] data nullptr or length = %u.", length);
        return;
    }
    LOGI("[NAdapt][OnDataRecv] Enter, from=%s{private}, length=%u", srcDevInfo.identifier.c_str(), length);
    {
        std::lock_guard<std::mutex> onReceiveLockGard(onReceiveMutex_);
        if (!onReceiveHandle_) {
            LOGE("[NAdapt][OnDataRecv] onReceiveHandle invalid.");
            return;
        }
        onReceiveHandle_(srcDevInfo.identifier, data, length);
    }
    // These code is compensation for the probable defect of IProcessCommunicator implementation.
    // As described in the agreement, for the missed online situation, we check the source dev when received.
    // OnDeviceChangeHandler is reused to check the existence of peer process.
    // Since this thread is the callback_thread of IProcessCommunicator, we do this check task directly in this thread.
    CheckDeviceOnlineAfterReception(srcDevInfo);
}

void NetworkAdapter::OnDeviceChangeHandler(const DeviceInfos &devInfo, bool isOnline)
{
    LOGI("[NAdapt][OnDeviceChange] Enter, dev=%s{private}, isOnline=%d", devInfo.identifier.c_str(), isOnline);
    // These code is compensation for the probable defect of IProcessCommunicator implementation.
    // As described in the agreement, for the mistake online situation, we check the existence of peer process.
    // The IProcessCommunicator implementation guarantee that no mistake offline will happen.
    if (isOnline) {
        if (!processCommunicator_->IsSameProcessLabelStartedOnPeerDevice(devInfo)) {
            LOGI("[NAdapt][OnDeviceChange] ######## Detect Not Really Online ########.");
            std::lock_guard<std::mutex> onlineRemoteDevLockGuard(onlineRemoteDevMutex_);
            onlineRemoteDev_.erase(devInfo.identifier);
            return;
        }
        std::lock_guard<std::mutex> onlineRemoteDevLockGuard(onlineRemoteDevMutex_);
        onlineRemoteDev_.insert(devInfo.identifier);
    } else {
        std::lock_guard<std::mutex> onlineRemoteDevLockGuard(onlineRemoteDevMutex_);
        onlineRemoteDev_.erase(devInfo.identifier);
    }
    // End compensation, do callback.
    std::lock_guard<std::mutex> onChangeLockGard(onChangeMutex_);
    if (!onChangeHandle_) {
        LOGE("[NAdapt][OnDeviceChange] onChangeHandle_ invalid.");
        return;
    }
    onChangeHandle_(devInfo.identifier, isOnline);
}

void NetworkAdapter::SearchOnlineRemoteDeviceAtStartup()
{
    std::vector<DeviceInfos> onlineDev = processCommunicator_->GetRemoteOnlineDeviceInfosList();
    LOGE("[NAdapt][SearchOnline] onlineDev count = %zu.", onlineDev.size());
    if (!onlineDev.empty()) {
        pendingAsyncTaskCount_.fetch_add(1);
        // Note: onlineDev should be captured by value (must not by reference)
        TaskAction callbackTask = [onlineDev, this]() {
            LOGI("[NAdapt][SearchOnline] Begin Callback In Async Task.");
            std::string localIdentity;
            GetLocalIdentity(localIdentity); // It doesn't matter if getlocal fail and localIdentity be an empty string
            for (auto &entry : onlineDev) {
                if (entry.identifier == localIdentity) {
                    LOGW("[NAdapt][SearchOnline] ######## Detect Local Device in Remote Device List ########.");
                    continue;
                }
                OnDeviceChangeHandler(entry, true);
            }
            pendingAsyncTaskCount_.fetch_sub(1);
            asyncTaskDoneCv_.notify_all();
            LOGI("[NAdapt][SearchOnline] End Callback In Async Task.");
        };
        // Use ScheduleQueuedTask to keep order
        int errCode = RuntimeContext::GetInstance()->ScheduleQueuedTask(SCHEDULE_QUEUE_TAG, callbackTask);
        if (errCode != E_OK) {
            LOGE("[NAdapt][SearchOnline] ScheduleQueuedTask failed, errCode = %d.", errCode);
            pendingAsyncTaskCount_.fetch_sub(1);
            asyncTaskDoneCv_.notify_all();
        }
    }
}

void NetworkAdapter::CheckDeviceOnlineAfterReception(const DeviceInfos &devInfo)
{
    bool isAlreadyOnline = true;
    {
        std::lock_guard<std::mutex> onlineRemoteDevLockGuard(onlineRemoteDevMutex_);
        if (onlineRemoteDev_.count(devInfo.identifier) == 0) {
            isAlreadyOnline = false;
        }
    }

    // Seem offline but receive data from it, let OnDeviceChangeHandler check whether it is really online
    if (!isAlreadyOnline) {
        OnDeviceChangeHandler(devInfo, true);
    }
}

void NetworkAdapter::CheckDeviceOfflineAfterSendFail(const DeviceInfos &devInfo)
{
    // Note: only the identifier field of devInfo is valid, enough to call IsSameProcessLabelStartedOnPeerDevice
    bool isAlreadyOffline = true;
    {
        std::lock_guard<std::mutex> onlineRemoteDevLockGuard(onlineRemoteDevMutex_);
        if (onlineRemoteDev_.count(devInfo.identifier) != 0) {
            isAlreadyOffline = false;
        }
    }

    // Seem online but send fail, we have to check whether still online
    if (!isAlreadyOffline) {
        if (!processCommunicator_->IsSameProcessLabelStartedOnPeerDevice(devInfo)) {
            LOGW("[NAdapt][CheckAfterSend] ######## Missed Offline Detected ########.");
            {
                // Mark this device not online immediately to avoid repeatedly miss-offline detect when send continually
                std::lock_guard<std::mutex> onlineRemoteDevLockGuard(onlineRemoteDevMutex_);
                onlineRemoteDev_.erase(devInfo.identifier);
            }
            pendingAsyncTaskCount_.fetch_add(1);
            // Note: devInfo should be captured by value (must not by reference)
            TaskAction callbackTask = [devInfo, this]() {
                LOGI("[NAdapt][CheckAfterSend] In Async Task, devInfo=%s{private}.", devInfo.identifier.c_str());
                OnDeviceChangeHandler(devInfo, false);
                pendingAsyncTaskCount_.fetch_sub(1);
                asyncTaskDoneCv_.notify_all();
            };
            // Use ScheduleQueuedTask to keep order
            int errCode = RuntimeContext::GetInstance()->ScheduleQueuedTask(SCHEDULE_QUEUE_TAG, callbackTask);
            if (errCode != E_OK) {
                LOGE("[NAdapt][CheckAfterSend] ScheduleQueuedTask failed, errCode = %d.", errCode);
                pendingAsyncTaskCount_.fetch_sub(1);
                asyncTaskDoneCv_.notify_all();
            }
        }
    }
}
}
