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
#include "ipc_server_stub.h"

#include "device_manager_errno.h"
#include "device_manager_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t IpcServerListener::SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    std::string pkgName = req->GetPkgName();
    sptr<IpcRemoteBroker> listener = IpcServerStub::GetInstance().GetDmListener(pkgName);
    if (listener == nullptr) {
        DMLOG(DM_LOG_INFO, "cannot get listener for package:%s.", pkgName.c_str());
        return DEVICEMANAGER_NULLPTR;
    }
    return listener->SendCmd(cmdCode, req, rsp);
}

int32_t IpcServerListener::SendAll(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    std::map<std::string, sptr<IRemoteObject>> listeners = IpcServerStub::GetInstance().GetDmListener();
    for (auto iter : listeners) {
        auto pkgName = iter.first;
        auto remote = iter.second;
        req->SetPkgName(pkgName);
        sptr<IpcRemoteBroker> listener = iface_cast<IpcRemoteBroker>(remote);
        listener->SendCmd(cmdCode, req, rsp);
    }
    return DEVICEMANAGER_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
