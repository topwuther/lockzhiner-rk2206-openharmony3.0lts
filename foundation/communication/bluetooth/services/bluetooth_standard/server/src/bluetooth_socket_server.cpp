/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "bluetooth_socket_server.h"
#include "bluetooth_log.h"
#include "interface_profile_manager.h"
#include "interface_profile_socket.h"

using namespace bluetooth;

namespace OHOS {
namespace Bluetooth {
int BluetoothSocketServer::Connect(std::string &addr, bluetooth::Uuid &uuid, int32_t securityFlag, int32_t type)
{
    HILOGI("BluetoothSocketServer: Connect starts");
    IProfileSocket *socket = (IProfileSocket *)IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP);

    return socket->Connect(addr, uuid, (int)securityFlag, (int)type);
}

int BluetoothSocketServer::Listen(std::string &name, bluetooth::Uuid &uuid, int32_t securityFlag, int32_t type)
{
    HILOGI("BluetoothSocketServer: Listen starts");
    IProfileSocket *socket = (IProfileSocket *)IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP);

    return socket->Listen(name, uuid, (int)securityFlag, (int)type);
}
}  // namespace Bluetooth
}  // namespace OHOS