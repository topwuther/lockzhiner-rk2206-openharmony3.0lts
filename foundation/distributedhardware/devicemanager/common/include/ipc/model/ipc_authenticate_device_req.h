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

#ifndef OHOS_DEVICE_MANAGER_IPC_AUTHENTICATE_DEVICE_REQ_H
#define OHOS_DEVICE_MANAGER_IPC_AUTHENTICATE_DEVICE_REQ_H

#include "ipc_req.h"

#include "dm_device_info.h"
#include "dm_app_image_info.h"

namespace OHOS {
namespace DistributedHardware {
class IpcAuthenticateDeviceReq : public IpcReq {
DECLARE_IPC_MODEL(IpcAuthenticateDeviceReq);
public:
    const DmDeviceInfo& GetDeviceInfo() const
    {
        return deviceInfo_;
    }

    void SetDeviceInfo(const DmDeviceInfo &deviceInfo)
    {
        deviceInfo_ = deviceInfo;
    }

    const DmAppImageInfo& GetAppImageInfo() const
    {
        return appImageInfo_;
    }

    void SetAppImageInfo(const DmAppImageInfo &appImageInfo)
    {
        appImageInfo_ = appImageInfo;
    }

    const std::string& GetExtra() const
    {
        return extra_;
    }

    void SetExtra(std::string &extra)
    {
        extra_ = extra;
    }
private:
    DmDeviceInfo deviceInfo_;
    DmAppImageInfo appImageInfo_;
    std::string extra_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_AUTHENTICATE_DEVICE_REQ_H
