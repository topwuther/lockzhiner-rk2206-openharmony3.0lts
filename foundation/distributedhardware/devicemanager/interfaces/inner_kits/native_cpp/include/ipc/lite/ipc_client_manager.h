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

#ifndef OHOS_DEVICE_MANAGER_IPC_CLIENT_MANAGER_H
#define OHOS_DEVICE_MANAGER_IPC_CLIENT_MANAGER_H

#include <cstdint>
#include <string>
#include <set>

#include "ipc_client.h"
#include "ipc_client_server_proxy.h"

namespace OHOS {
namespace DistributedHardware {
class IpcClientManager : public IpcClient {
DECLARE_IPC_INTERFACE(IpcClientManager);
public:
    virtual int32_t Init(std::string &pkgName) override;
    virtual int32_t UnInit(std::string &pkgName) override;
    virtual int32_t SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp) override;
private:
    bool IsInit(std::string &pkgName);
private:
    IpcClientServerProxy serverProxy_;
    std::set<std::string> packageInitSet_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_CLIENT_MANAGER_H