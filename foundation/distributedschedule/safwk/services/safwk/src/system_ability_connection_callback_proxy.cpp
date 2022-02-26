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

#include "system_ability_connection_callback_proxy.h"

#include "errors.h"
#include "ipc_skeleton.h"
#include "iremote_proxy.h"
#include "safwk_log.h"
#include "string_ex.h"

namespace OHOS {
namespace {
const std::string TAG = "SystemAbilityConnectionCallbackProxy";

const std::u16string SA_CONNECTION_CALLBACK_INTERFACE_TOKEN = u"ohos.distributedschedule.saconnectAccessToken";
}

void SystemAbilityConnectionCallbackProxy::OnConnectedSystemAbility(const sptr<IRemoteObject>& connectionCallback)
{
    HILOGI(TAG, "system ability connected");
    if (connectionCallback == nullptr) {
        HILOGW(TAG, "connectionCallback is null!");
        return;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOGW(TAG, "remote is nullptr!");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SA_CONNECTION_CALLBACK_INTERFACE_TOKEN)) {
        HILOGW(TAG, "write interface token failed!");
        return;
    }

    if (!data.WriteRemoteObject(connectionCallback)) {
        HILOGW(TAG, "write connectionCallback failed!");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(ON_CONNECTED_SYSTEM_ABILITY, data, reply, option);
    HILOGI(TAG, "SendRequest ret:%{public}d", ret);
}

void SystemAbilityConnectionCallbackProxy::OnDisConnectedSystemAbility(int32_t systemAbilityId)
{
    HILOGI(TAG, "systemAbilityId:%{public}d", systemAbilityId);
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGW(TAG, "remote is nullptr!");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(SA_CONNECTION_CALLBACK_INTERFACE_TOKEN)) {
        return;
    }

    if (!data.WriteInt32(systemAbilityId)) {
        HILOGW(TAG, "write systemAbilityId failed!");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(ON_DISCONNECTED_SYSTEM_ABILITY, data, reply, option);
    HILOGI(TAG, "SendRequest ret:%{public}d", ret);
}
} // namespace OHOS
