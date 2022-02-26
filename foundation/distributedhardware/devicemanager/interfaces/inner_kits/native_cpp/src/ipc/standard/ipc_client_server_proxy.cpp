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

#include "ipc_client_server_proxy.h"

#include "ipc_cmd_register.h"
#include "ipc_types.h"

#include "device_manager_log.h"
#include "device_manager_errno.h"

namespace OHOS {
namespace DistributedHardware {
int32_t IpcClientServerProxy::SendCmd(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DMLOG(DM_LOG_ERROR, "remote service null");
        return DEVICEMANAGER_NULLPTR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_FAILED;
    }
    if (remote->SendRequest(cmdCode, data, reply, option) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "SendRequest fail, cmd:%d", cmdCode);
        return DEVICEMANAGER_IPC_FAILED;
    }
    return IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
}
} // namespace DistributedHardware
} // namespace OHOS
