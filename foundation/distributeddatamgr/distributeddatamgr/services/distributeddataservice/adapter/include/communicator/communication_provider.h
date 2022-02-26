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

#ifndef DISTRIBUTEDDATA_COMMUNICATION_PROVIDER_H
#define DISTRIBUTEDDATA_COMMUNICATION_PROVIDER_H

#include <memory>
#include <vector>
#include "app_data_change_listener.h"
#include "app_device_status_change_listener.h"
#include "app_types.h"
#include "idevice_query.h"
#include "visibility.h"
namespace OHOS {
namespace AppDistributedKv {
class CommunicationProvider {
public:
    // constructor
    KVSTORE_API CommunicationProvider() {};

    // destructor
    KVSTORE_API virtual ~CommunicationProvider() {};

    // add DeviceChangeListener to watch device change
    KVSTORE_API
    virtual Status StartWatchDeviceChange(const AppDeviceStatusChangeListener *observer, const PipeInfo &pipeInfo) = 0;

    // stop DeviceChangeListener to watch device change
    KVSTORE_API
    virtual Status StopWatchDeviceChange(const AppDeviceStatusChangeListener *observer, const PipeInfo &pipeInfo) = 0;

    // add DataChangeListener to watch data change
    KVSTORE_API
    virtual Status StartWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo) = 0;

    // stop DataChangeListener to watch data change
    KVSTORE_API virtual Status StopWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo) = 0;

    // Send data to other device, function will be called back after sent to notify send result
    KVSTORE_API
    virtual Status SendData(const PipeInfo &pipeInfo, const DeviceId &deviceId, const uint8_t *ptr, int size,
                            const MessageInfo &info = {MessageType::DEFAULT}) = 0;

    // Get online deviceList
    KVSTORE_API virtual std::vector<DeviceInfo> GetDeviceList() const = 0;

    // Get local device information
    KVSTORE_API virtual DeviceInfo GetLocalDevice() const = 0;

    // start one server to listen data from other devices;
    KVSTORE_API virtual Status Start(const PipeInfo &pipeInfo) = 0;

    // stop server
    KVSTORE_API virtual Status Stop(const PipeInfo &pipeInfo) = 0;

    // user should use this method to get instance of CommunicationProvider;
    KVSTORE_API static CommunicationProvider &GetInstance();

    KVSTORE_API static std::shared_ptr<CommunicationProvider> MakeCommunicationProvider();

    // check peer device pipeInfo Process
    KVSTORE_API virtual bool IsSameStartedOnPeer(const PipeInfo &pipeInfo, const DeviceId &peer) const = 0;

    KVSTORE_API virtual void SetDeviceQuery(std::shared_ptr<IDeviceQuery> deviceQuery) = 0;
    KVSTORE_API virtual std::string GetUuidByNodeId(const std::string &nodeId) const = 0;
    KVSTORE_API virtual std::string GetUdidByNodeId(const std::string &nodeId) const = 0;
    KVSTORE_API virtual DeviceInfo GetLocalBasicInfo() const = 0;
    KVSTORE_API virtual std::vector<DeviceInfo> GetRemoteNodesBasicInfo() const = 0;
    KVSTORE_API virtual std::string ToNodeId(const std::string &id) const = 0;

    KVSTORE_API virtual void SetMessageTransFlag(const PipeInfo &pipeInfo, bool flag) = 0;
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATA_COMMUNICATION_PROVIDER_H
