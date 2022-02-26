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

#ifndef OHOS_DEVICE_MANAGER_IMPL_H
#define OHOS_DEVICE_MANAGER_IMPL_H

#include <memory>

#include "device_manager.h"

#include "ipc_client_proxy.h"
#include "ipc_client_manager.h"

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerImpl : public DeviceManager {
DECLARE_SINGLE_INSTANCE(DeviceManagerImpl);
public:
    virtual int32_t InitDeviceManager(std::string &pkgName,
        std::shared_ptr<DmInitCallback> dmInitCallback) override;
    virtual int32_t UnInitDeviceManager(std::string &pkgName) override;
    virtual int32_t GetTrustedDeviceList(std::string &pkgName, std::string &extra,
        std::vector<DmDeviceInfo> &deviceList) override;
    virtual int32_t RegisterDevStateCallback(std::string &pkgName, std::string &extra,
        std::shared_ptr<DeviceStateCallback> callback) override;
    virtual int32_t UnRegisterDevStateCallback(std::string &pkgName) override;
    virtual int32_t StartDeviceDiscovery(std::string &pkgName, DmSubscribeInfo &subscribeInfo,
        std::shared_ptr<DiscoverCallback> callback) override;
    virtual int32_t StopDeviceDiscovery(std::string &pkgName, uint16_t subscribeId) override;
    virtual int32_t AuthenticateDevice(std::string &pkgName, const DmDeviceInfo &deviceInfo,
        const DmAppImageInfo &imageInfo, std::string &extra,
        std::shared_ptr<AuthenticateCallback> callback) override;
    virtual int32_t CheckAuthentication(std::string &pkgName, std::string &authPara,
        std::shared_ptr<CheckAuthCallback> callback) override;
    virtual int32_t GetAuthenticationParam(std::string &pkgName, DmAuthParam &authParam) override;
    virtual int32_t SetUserOperation(std::string &pkgName, int32_t action) override;
    virtual int32_t RegisterDeviceManagerFaCallback(std::string &packageName,
        std::shared_ptr<DeviceManagerFaCallback> callback) override;
    virtual int32_t UnRegisterDeviceManagerFaCallback(std::string &pkgName) override;
private:
    std::shared_ptr<IpcClientProxy> ipcClientProxy_ =
        std::make_shared<IpcClientProxy>(std::make_shared<IpcClientManager>());
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IMPL_H
