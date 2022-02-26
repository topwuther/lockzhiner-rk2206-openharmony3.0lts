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

#ifndef OHOS_DEVICE_MANAGER_IPC_NOTIFY_AUTH_RESULT_REQ_H
#define OHOS_DEVICE_MANAGER_IPC_NOTIFY_AUTH_RESULT_REQ_H

#include <cstdint>

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyAuthResultReq : public IpcReq {
DECLARE_IPC_MODEL(IpcNotifyAuthResultReq);
public:
    std::string GetDeviceId() const
    {
        return deviceId_;
    }

    void SetDeviceId(std::string& deviceId)
    {
        deviceId_ = deviceId;
    }

    int32_t GetPinToken() const
    {
        return pinToken_;
    }

    void SetPinToken(int32_t pinToken)
    {
        pinToken_ = pinToken;
    }

    int32_t GetStatus() const
    {
        return status_;
    }

    void SetStatus(int32_t status)
    {
        status_ = status;
    }

    int32_t GetReason() const
    {
        return reason_;
    }

    void SetReason(int32_t reason)
    {
        reason_ = reason;
    }
private:
    std::string deviceId_;
    int32_t pinToken_ {0};
    int32_t status_ {0};
    int32_t reason_ {0};
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_NOTIFY_AUTH_RESULT_REQ_H
