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

#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "device_manager_notify.h"

#include "ipc_def.h"
#include "ipc_cmd_register.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(IpcClientStub);

static int32_t ClientIpcInterfaceMsgHandle(const IpcContext *ctx, void *ipcMsg, IpcIo *io, void *arg)
{
    (void)arg;
    if (ipcMsg == nullptr || io == nullptr || ctx == nullptr) {
        DMLOG(DM_LOG_ERROR, "invalid param");
        return DEVICEMANAGER_INVALID_PARAM;
    }

    uint32_t code = 0;
    GetCode(ipcMsg, &code);
    int32_t errCode = DEVICEMANAGER_OK;

    errCode = IpcCmdRegister::GetInstance().OnIpcCmd(code, *io);
    DMLOG(DM_LOG_INFO, "receive ipc transact code:%u, retCode=%d", code, errCode);
    FreeBuffer(ctx, ipcMsg);
    return errCode;
}

int32_t IpcClientStub::Init()
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (bInit) {
        return DEVICEMANAGER_OK;
    }
    if (RegisterIpcCallback(ClientIpcInterfaceMsgHandle, 0, IPC_WAIT_FOREVER, &clientIdentity_, nullptr) != 0) {
        DMLOG(DM_LOG_ERROR, "register ipc cb failed");
        return DEVICEMANAGER_FAILED;
    }
    bInit = true;
    return DEVICEMANAGER_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
