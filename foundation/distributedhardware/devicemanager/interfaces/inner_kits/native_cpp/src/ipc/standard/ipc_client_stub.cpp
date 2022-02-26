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

#include "ipc_client_stub.h"

#include "ipc_skeleton.h"
#include "ipc_types.h"

#include "ipc_cmd_register.h"

#include "device_manager_log.h"
#include "device_manager_errno.h"

namespace OHOS {
namespace DistributedHardware {
int32_t IpcClientStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DMLOG(DM_LOG_INFO, "code = %d, flags= %d.", code, option.GetFlags());
    if (IpcCmdRegister::GetInstance().OnIpcCmd(code, data, reply) == DEVICEMANAGER_OK) {
        return DEVICEMANAGER_OK;
    }
    DMLOG(DM_LOG_WARN, "unsupport code: %d", code);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t IpcClientStub::SendCmd(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    DMLOG(DM_LOG_ERROR, "error");
    return DEVICEMANAGER_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
