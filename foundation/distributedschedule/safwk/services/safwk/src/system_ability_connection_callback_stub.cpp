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

#include "system_ability_connection_callback_stub.h"

#include "errors.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "safwk_log.h"

namespace OHOS {
namespace {
const std::string TAG = "SystemAbilityConnectionCallbackStub";

const std::u16string SA_CONNECTION_CALLBACK_INTERFACE_TOKEN = u"ohos.distributedschedule.saconnectAccessToken";
}

int32_t SystemAbilityConnectionCallbackStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    HILOGI(TAG, "code:%{public}d, flags:%{public}d", code, option.GetFlags());

    if (data.ReadInterfaceToken() != SA_CONNECTION_CALLBACK_INTERFACE_TOKEN) {
        return ERR_PERMISSION_DENIED;
    }

    switch (code) {
        case ON_CONNECTED_SYSTEM_ABILITY: {
            auto object = data.ReadRemoteObject();
            if (object == nullptr) {
                HILOGW(TAG, "ReadRemoteObject failed!");
                return ERR_NULL_OBJECT;
            }

            OnConnectedSystemAbility(object);
            return ERR_OK;
        }
        case ON_DISCONNECTED_SYSTEM_ABILITY: {
            int32_t systemAbilityId = data.ReadInt32();
            if (systemAbilityId <= 0) {
                HILOGW(TAG, "read saId failed!");
                return ERR_NULL_OBJECT;
            }

            OnDisConnectedSystemAbility(systemAbilityId);
            return ERR_OK;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}
} // namespace OHOS
