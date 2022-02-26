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

#ifndef NETWORK_ADAPTER_H
#define NETWORK_ADAPTER_H

#include <set>
#include <map>
#include <mutex>
#include <atomic>
#include <memory>
#include <condition_variable>
#include "iadapter.h"
#include "iprocess_communicator.h"

namespace DistributedDB {
class NetworkAdapter : public IAdapter {
public:
    NetworkAdapter();
    NetworkAdapter(const std::string &inProcessLabel);
    NetworkAdapter(const std::string &inProcessLabel, const std::shared_ptr<IProcessCommunicator> &inCommunicator);

    ~NetworkAdapter() override;

    int StartAdapter() override;
    void StopAdapter() override;

    uint32_t GetMtuSize() override;
    uint32_t GetMtuSize(const std::string &target) override;
    int GetLocalIdentity(std::string &outTarget) override;

    int SendBytes(const std::string &dstTarget, const uint8_t *bytes, uint32_t length) override;

    int RegBytesReceiveCallback(const BytesReceiveCallback &onReceive, const Finalizer &inOper) override;
    int RegTargetChangeCallback(const TargetChangeCallback &onChange, const Finalizer &inOper) override;
    int RegSendableCallback(const SendableCallback &onSendable, const Finalizer &inOper) override;
private:
    void OnDataReceiveHandler(const DeviceInfos &srcDevInfo, const uint8_t *data, uint32_t length);
    void OnDeviceChangeHandler(const DeviceInfos &devInfo, bool isOnline);

    void SearchOnlineRemoteDeviceAtStartup();
    void CheckDeviceOnlineAfterReception(const DeviceInfos &devInfo);
    void CheckDeviceOfflineAfterSendFail(const DeviceInfos &devInfo);

    std::string processLabel_;
    std::shared_ptr<IProcessCommunicator> processCommunicator_;

    // For protecting "LocalIdentity" and "MtuSize", these info only need to get from peripheral interface once
    mutable std::mutex identityMutex_;
    bool isLocalIdentityValid_ = false;
    std::string localIdentity_;
    mutable std::mutex mtuSizeMutex_;
    bool isMtuSizeValid_ = false;
    uint32_t mtuSize_ = 0;
    std::map<std::string, uint32_t> devMapMtuSize_;

    mutable std::mutex onlineRemoteDevMutex_;
    std::set<std::string> onlineRemoteDev_; // Refer to devices that has peer process

    std::atomic<int> pendingAsyncTaskCount_{0};
    mutable std::mutex asyncTaskDoneMutex_;
    std::condition_variable asyncTaskDoneCv_;

    BytesReceiveCallback onReceiveHandle_;
    TargetChangeCallback onChangeHandle_;
    SendableCallback onSendableHandle_;
    Finalizer onReceiveFinalizer_;
    Finalizer onChangeFinalizer_;
    Finalizer onSendableFinalizer_;
    mutable std::mutex onReceiveMutex_;
    mutable std::mutex onChangeMutex_;
    mutable std::mutex onSendableMutex_;
};
} // namespace DistributedDB

#endif
