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

#include "ipc_server_listener.h"

#include "device_manager_log.h"
#include "device_manager_errno.h"

#include "ipc_cmd_register.h"
#include "ipc_def.h"
#include "ipc_server_listenermgr.h"
#include "ipc_server_adapter.h"

namespace OHOS {
namespace DistributedHardware {
void IpcServerListener::CommonSvcToIdentity(CommonSvcId *svcId, SvcIdentity *identity)
{
    identity->handle = svcId->handle;
    identity->token = svcId->token;
    identity->cookie = svcId->cookie;
#ifdef __LINUX__
    identity->ipcContext = svcId->ipcCtx;
#endif
}

int32_t IpcServerListener::GetIdentityByPkgName(std::string &name, SvcIdentity *svc)
{
    CommonSvcId svcId;
    if (IpcServerListenermgr::GetInstance().GetListenerByPkgName(name, &svcId) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "get identity failed.");
        return DEVICEMANAGER_FAILED;
    }
    CommonSvcToIdentity(&svcId, svc);
    return DEVICEMANAGER_OK;
}

int32_t IpcServerListener::SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    std::string pkgName = req->GetPkgName();
    SvcIdentity svc;
    if (GetIdentityByPkgName(pkgName, &svc) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "ondevice found callback get listener failed.");
        return DEVICEMANAGER_FAILED;
    }

    IpcIo io;
    uint8_t data[MAX_DM_IPC_LEN] = {0};
    if (IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, io, data, MAX_DM_IPC_LEN) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_DEBUG, "SetRequest failed cmdCode:%d", cmdCode);
        return DEVICEMANAGER_FAILED;
    }
    if (::SendRequest(nullptr, svc, cmdCode, &io, nullptr, LITEIPC_FLAG_ONEWAY, nullptr) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_DEBUG, "SendRequest failed cmdCode:%d", cmdCode);
    }
    return DEVICEMANAGER_OK;
}

int32_t IpcServerListener::SendAll(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    const std::map<std::string, CommonSvcId> &listenerMap = IpcServerListenermgr::GetInstance().GetAllListeners();
    for (auto &kv : listenerMap) {
        SvcIdentity svc;
        IpcIo io;
        uint8_t data[MAX_DM_IPC_LEN] = {0};
        std::string pkgName = kv.first;

        req->SetPkgName(pkgName);
        if (IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, io, data, MAX_DM_IPC_LEN) != DEVICEMANAGER_OK) {
            DMLOG(DM_LOG_DEBUG, "SetRequest failed cmdCode:%d", cmdCode);
            continue;
        }
        CommonSvcId svcId = kv.second;
        CommonSvcToIdentity(&svcId, &svc);
        if (::SendRequest(nullptr, svc, cmdCode, &io, nullptr, LITEIPC_FLAG_ONEWAY, nullptr) != DEVICEMANAGER_OK) {
            DMLOG(DM_LOG_DEBUG, "SendRequest failed cmdCode:%d", cmdCode);
        }
    }
    return DEVICEMANAGER_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
