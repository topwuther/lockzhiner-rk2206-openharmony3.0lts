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

#ifndef OHOS_DEVICE_MANAGER_IPC_SERVER_LISTENER_ADAPTER_H
#define OHOS_DEVICE_MANAGER_IPC_SERVER_LISTENER_ADAPTER_H

#include "discovery_service.h"

#include "ipc_server_listener.h"

#include "single_instance.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
class IpcServerListenerAdapter {
DECLARE_SINGLE_INSTANCE(IpcServerListenerAdapter);
public:
    void OnDeviceStateChange(DmDeviceState state, DmDeviceInfo &deviceInfo);
    void OnDeviceFound(std::string &pkgName, uint16_t originId, DmDeviceInfo &deviceInfo);
    void OnDiscoverFailed(std::string &pkgName, uint16_t originId, DiscoveryFailReason failReason);
    void OnDiscoverySuccess(std::string &pkgName, uint16_t originId);
    void OnAuthResult(std::string &pkgName, std::string &deviceId, int32_t pinToken, uint32_t status, uint32_t reason);
    void OnCheckAuthResult(std::string &deviceId, int32_t resultCode, int32_t flag);
    void OnFaCall(std::string &pkgName, std::string &paramJson);
private:
    IpcServerListener ipcServerListener_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_SERVER_LISTENER_ADAPTER_H
