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

#ifndef PROCESS_COMMUNICATOR_IMPL_H
#define PROCESS_COMMUNICATOR_IMPL_H

#include <mutex>
#include "iprocess_communicator.h"
#include "communication_provider.h"

namespace OHOS {
namespace AppDistributedKv {
class ProcessCommunicatorImpl : public DistributedDB::IProcessCommunicator,
                                private AppDataChangeListener,
                                private AppDeviceStatusChangeListener {
public:
    using DBStatus = DistributedDB::DBStatus;
    using OnDeviceChange = DistributedDB::OnDeviceChange;
    using OnDataReceive = DistributedDB::OnDataReceive;
    using DeviceInfos = DistributedDB::DeviceInfos;
    KVSTORE_API ProcessCommunicatorImpl();
    KVSTORE_API ~ProcessCommunicatorImpl() override;

    KVSTORE_API DBStatus Start(const std::string &processLabel) override;
    KVSTORE_API DBStatus Stop() override;

    KVSTORE_API DBStatus RegOnDeviceChange(const OnDeviceChange &callback) override;
    KVSTORE_API DBStatus RegOnDataReceive(const OnDataReceive &callback) override;

    KVSTORE_API DBStatus SendData(const DeviceInfos &dstDevInfo, const uint8_t *data, uint32_t length) override;
    KVSTORE_API uint32_t GetMtuSize() override;
    KVSTORE_API uint32_t GetMtuSize(const DeviceInfos &devInfo) override;
    KVSTORE_API DeviceInfos GetLocalDeviceInfos() override;
    KVSTORE_API std::vector<DeviceInfos> GetRemoteOnlineDeviceInfosList() override;
    KVSTORE_API bool IsSameProcessLabelStartedOnPeerDevice(const DeviceInfos &peerDevInfo) override;
private:
    void OnMessage(const DeviceInfo &info, const uint8_t *ptr, const int size,
                   const PipeInfo &pipeInfo) const override;
    void OnDeviceChanged(const DeviceInfo &info, const DeviceChangeType &type) const override;

    std::string thisProcessLabel_;
    OnDeviceChange onDeviceChangeHandler_;
    OnDataReceive onDataReceiveHandler_;
    mutable std::mutex onDeviceChangeMutex_;
    mutable std::mutex onDataReceiveMutex_;

    static constexpr uint32_t MTU_SIZE = 4096; // the max transmission unit size(4K - 80B)
    static constexpr uint32_t MTU_SIZE_WATCH = 81920; // the max transmission unit size(80K)
    static constexpr const char *SMART_WATCH_TYPE = "SMART_WATCH";
    static constexpr const char *CHILDREN_WATCH_TYPE = "CHILDREN_WATCH";
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif // PROCESS_COMMUNICATOR_IMPL_H
