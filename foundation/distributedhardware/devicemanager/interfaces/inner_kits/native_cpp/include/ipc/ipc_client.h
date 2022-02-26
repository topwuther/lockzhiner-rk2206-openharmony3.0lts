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

#ifndef OHOS_DEVICE_MANAGER_IPC_CLIENT_H
#define OHOS_DEVICE_MANAGER_IPC_CLIENT_H

#include <cstdint>
#include <string>
#include <memory>

#include "ipc_req.h"
#include "ipc_rsp.h"

namespace OHOS {
namespace DistributedHardware {
class IpcClient {
DECLARE_IPC_INTERFACE(IpcClient);
public:
    virtual int32_t Init(std::string &pkgName) = 0;
    virtual int32_t UnInit(std::string &pkgName) = 0;
    virtual int32_t SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_CLIENT_H