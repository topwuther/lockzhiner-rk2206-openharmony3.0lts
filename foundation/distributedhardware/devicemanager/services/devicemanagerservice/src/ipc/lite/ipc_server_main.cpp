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

#include <cstdint>
#include <unistd.h>

#include "device_manager_log.h"
#include "device_manager_errno.h"

#include "ipc_server_stub.h"
#include "ipc_server_adapter.h"

using namespace OHOS::DistributedHardware;

static void InitAll()
{
    const int32_t DM_SERVICE_INIT_DELAY = 5;

    sleep(DM_SERVICE_INIT_DELAY);
    if (IpcServerStubInit() != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "IpcServerStubInit failed");
        return;
    }
    if (IpcServerAdapter::GetInstance().ModuleInit() != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "module init failed");
        return;
    }
    DMLOG(DM_LOG_INFO, "DM ipc server Init success");
}

int32_t main(int32_t argc, char *argv[])
{
    (void)argc;
    (void)argv;
    InitAll();
    while (1) {
        pause();
    }
    return 0;
}